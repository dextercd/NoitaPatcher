#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

void receive_all(sf::TcpSocket& sock, void* data, std::size_t size)
{
    auto const restore_blocking = sock.isBlocking();
    sock.setBlocking(true);

    while (size) {
        std::size_t received{};
        auto result = sock.receive(data, size, received);
        if (result != sf::Socket::Status::Done && result != sf::Socket::Status::NotReady) {
            throw std::runtime_error{"Read error"};
        }

        size -= received;
        data = reinterpret_cast<char*>(data) + received;
    }

    sock.setBlocking(restore_blocking);
}

sf::Socket::Status receive_all_or_none(sf::TcpSocket& sock, void* data, std::size_t size)
{
    std::size_t received{};
    auto result = sock.receive(data, size, received);

    if (received == 0)
        return sf::Socket::Status::NotReady;

    auto left = size - received;
    if (left) {
        data = reinterpret_cast<char*>(data) + received;
        receive_all(sock, data, left);
    }

    return sf::Socket::Status::Done;
}

std::uint32_t read_le_uint(char* buf)
{
    return (unsigned char)buf[0]
         | (unsigned char)buf[1] << 8
         | (unsigned char)buf[2] << 16
         | (unsigned char)buf[3] << 24;
}

std::int32_t read_le_int(char* buf)
{
    return read_le_uint(buf);
}

sf::Socket::Status sock_read_int(sf::TcpSocket& sock, std::uint32_t& res)
{
    char buffer[4];
    auto recv_result = receive_all_or_none(sock, buffer, sizeof(buffer));

    if (recv_result != sf::Socket::Status::Done) {
        res = 0;
        return recv_result;
    }

    res = read_le_uint(buffer);
    return sf::Socket::Status::Done;
}

std::vector<sf::Color> get_random_colors()
{
    std::vector<sf::Color> c;
    for (int i = 0; i != 50000; ++i)
        c.push_back(sf::Color(std::rand(), std::rand(), std::rand()));
    return c;
}

sf::Color get_color(int index)
{
    static auto colors = get_random_colors();
    return colors[index];
}


void run(sf::TcpSocket& client)
{
    double camx = 0;
    double camy = 0;
    double camz = 100;

    std::vector<char> pixel_data_buf;

    client.setBlocking(false);

    std::cout << "Creating window\n" << std::flush;
    sf::RenderWindow window{sf::VideoMode{800, 600}, "Noita Map"};
    std::cout << "Window created\n" << std::flush;

    sf::Image map_image;
    map_image.create(4096, 4096);
    auto image_size = map_image.getSize();

    sf::Texture map_texture;
    map_texture.loadFromImage(map_image);
    auto tex_size = map_texture.getSize();

    sf::Sprite sprite{map_texture};

    int received_nothing = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) camx -= 2;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) camx += 2;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) camy -= 2;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) camy += 2;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Add)) camz -= 2;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Subtract)) camz += 2;

        sf::View v{sf::Vector2f(camx, camy), sf::Vector2f(camz, camz)};
        window.setView(v);

        bool any_updates = false;
        bool data_left = true;

        while (data_left) {
            char dimens_data[4 * sizeof(std::int32_t)];
            data_left = receive_all_or_none(client, dimens_data, sizeof(dimens_data)) == sf::Socket::Status::Done;

            if (!data_left) {
                received_nothing = std::min(120, received_nothing + 1);
                auto sleep_for = std::max(0, received_nothing - 10) * 15;
                if (sleep_for > 0) {
                    std::this_thread::sleep_for(
                        std::chrono::microseconds{sleep_for});
                }
                break;
            }

            any_updates = true;
            received_nothing = 0;

            auto start_x = read_le_int(dimens_data + 0);
            auto start_y = read_le_int(dimens_data + 4);
            auto end_x = read_le_int(dimens_data + 8);
            auto end_y = read_le_int(dimens_data + 12);

            auto width = end_x - start_x;
            auto height = end_y - start_y;
            auto area = width * height;


            pixel_data_buf.resize(2 * area);
            receive_all(client, &pixel_data_buf[0], std::size(pixel_data_buf));

            std::size_t ix = 0;
            for (int y = start_y; y != end_y; ++y) {
                for (int x = start_x; x != end_x; ++x) {
                    char* data = &pixel_data_buf[ix * 2];

                    map_image.setPixel(
                        (x + (int)image_size.x / 2) % image_size.x,
                        (y + (int)image_size.y / 2) % image_size.y,
                        get_color(*reinterpret_cast<std::int16_t*>(data))
                    );

                    ++ix;
                }
            }
        }

        if (any_updates)
            map_texture.loadFromImage(map_image);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    client.disconnect();
}

int main()
{
    // Create a listener socket and make it wait for new
    sf::TcpListener listener;
    listener.listen(44174);

    // Endless loop that waits for new connections
    while (true)
    {
        sf::TcpSocket client;
        if (listener.accept(client) == sf::Socket::Done)
        {
            // A new client just connected!
            std::cout << "New connection received from " << client.getRemoteAddress() << std::endl;
            run(client);
        }
    }
}
