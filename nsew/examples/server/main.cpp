#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <vector>
#include <deque>
#include <iostream>

#include <boost/asio.hpp>

#include <absl/hash/hash.h>
#include <absl/container/flat_hash_map.h>

#include "nsew/serialise.hpp"

namespace basio = boost::asio;
using btcp = boost::asio::ip::tcp;
namespace bsys = boost::system;

enum class material_code : std::int16_t {
    unknown = -1,
    air = 0,
};

struct cell {
    material_code material{material_code::unknown};
};

constexpr std::int32_t chunk_extent = 512;

struct chunk_data {
    cell cells[chunk_extent * chunk_extent];

    cell index(int x, int y) const
    {
        return cells[y * chunk_extent + x];
    }
};

struct world_coord {
    std::int32_t x;
    std::int32_t y;

    friend bool operator==(const world_coord&, const world_coord&) = default;

    template<typename H>
    friend H AbslHashValue(H h, world_coord c) {
        return H::combine(std::move(h), c.x, c.y);
    }
};

struct chunk_coord {
    std::int32_t x;
    std::int32_t y;

    friend bool operator==(const chunk_coord&, const chunk_coord&) = default;

    template<typename H>
    friend H AbslHashValue(H h, chunk_coord c) {
        return H::combine(std::move(h), c.x, c.y);
    }

    constexpr explicit chunk_coord(world_coord w)
        : x{w.x / chunk_extent}
        , y{w.y / chunk_extent}
    {
    }
};

enum class chunk_status {
    unseen,
    loaded,
    stored,
};

struct world_data {
    absl::flat_hash_map<chunk_coord, std::unique_ptr<chunk_data>> chunks;

    chunk_status status_at(chunk_coord cc)
    {
        return chunk_status::unseen;
    }

    chunk_data* chunk_at(chunk_coord cc)
    {
        if (auto f = chunks.find(cc); f != std::end(chunks))
            return f->second.get();

        return nullptr;
    }

    chunk_data* get_chunk(chunk_coord cc)
    {
        auto& chunk_slot = chunks[cc];
        if (!chunk_slot)
            chunk_slot = std::make_unique<chunk_data>();

        return chunk_slot.get();
    }
};

class player {
public:
    virtual void send_world_data(std::vector<char>) = 0;
};

class world_session {
    world_data world;
    std::vector<std::shared_ptr<player>> players;

public:
    void join(std::shared_ptr<player> new_player)
    {
        players.push_back(std::move(new_player));
    }

    void submit_world_data(std::shared_ptr<player> submitter, std::vector<char> data)
    {
        for (auto& player : players) {
            if (player == submitter)
                continue;

            player->send_world_data(data);
        }
    }
};

struct header {
    world_coord top_left;
    std::uint8_t width;
    std::uint8_t height;
    std::uint16_t pixel_run_count;

    static constexpr header from_buffer(char const* buffer)
    {
        return header{
            {
                nsew::serialise<std::int32_t>::read(buffer),
                nsew::serialise<std::int32_t>::read(buffer + 4),
            },
            nsew::serialise<std::uint8_t>::read(buffer + 8),
            nsew::serialise<std::uint8_t>::read(buffer + 9),
            nsew::serialise<std::uint16_t>::read(buffer + 10),
        };
    }
};

constexpr int header_size = sizeof(header);

class player_session
    : public player
    , public std::enable_shared_from_this<player_session>
{
    btcp::socket socket;
    world_session& universe;
    std::vector<char> receive_buffer;
    std::deque<std::vector<char>> write_queue;

public:
    player_session(btcp::socket sock, world_session& uni)
        : socket{std::move(sock)}
        , universe{uni}
    {
    }

    void start()
    {
        universe.join(shared_from_this());
        do_read();
    }

    void do_read()
    {
        receive_buffer.resize(header_size);
        basio::async_read(
            socket,
            basio::buffer(receive_buffer.data(), header_size),
            [this, self=shared_from_this()](bsys::error_code ec, std::size_t)
            {
                if (ec) {
                    std::cerr << ec << '\n';
                    std::abort();
                }

                auto msg_header = header::from_buffer(receive_buffer.data());
                do_read_body(msg_header);
            });
    }

    void do_read_body(header msg_header)
    {
        std::cout
            << msg_header.top_left.x << ", " << msg_header.top_left.y << ": "
            << (int)msg_header.width << ", " << (int)msg_header.height << '\n';

        auto const body_size = msg_header.pixel_run_count * 5;
        receive_buffer.resize(header_size + body_size);

        basio::async_read(
            socket,
            basio::buffer(receive_buffer.data() + header_size, body_size),
            [this, self=shared_from_this()](bsys::error_code ec, std::size_t)
            {
                universe.submit_world_data(self, receive_buffer);
                do_read();
            });
    }

    void send_world_data(std::vector<char> data) override
    {
        bool write_in_progress = !write_queue.empty();
        write_queue.push_back(std::move(data));

        if (!write_in_progress)
            do_write();
    }

    void do_write()
    {
        basio::async_write(socket,
            basio::buffer(write_queue.front(), write_queue.front().size()),
            [this, self=shared_from_this()](bsys::error_code ec, std::size_t)
            {
                if (ec) {
                    std::cerr << ec << '\n';
                    std::abort();
                }

                write_queue.pop_front();
                if (!write_queue.empty())
                    do_write();
            });
    }
};

class server {
    btcp::acceptor acceptor;
    world_session universe;

    void do_accept()
    {
        acceptor.async_accept(
            [this](bsys::error_code ec, btcp::socket socket) {
                if (!ec) {
                    std::cout << "New connection: " << socket.remote_endpoint() << '\n';
                    auto new_player = std::make_shared<player_session>(
                        std::move(socket), universe);
                    new_player->start();
                } else {
                    std::cerr << ec << '\n';
                }

                do_accept();
            });
    }

public:
    server(basio::io_context& io, btcp::endpoint endpoint)
        : acceptor{io, endpoint}
    {
        do_accept();
    }
};

int main()
{
    basio::io_context io_context;

    auto serv = server{io_context, {btcp::v4(), 44174}};

    io_context.run();
}
