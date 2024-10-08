#include <httplib.h>

#include <exception>
#include <glaze/glaze.hpp>

#include <equicord/logger.hpp>
#include <equicord/patchbay.hpp>

int main(int argc, char **args)
{
    using equicord::logger;
    using equicord::patchbay;

    auto port = 7591;

    if (argc > 1)
    {
        try
        {
            port = std::stoi(args[1]);
        }
        catch (...)
        {
            logger::get()->error("Bad arguments, usage: {} [port]", args[0]);
            return 1;
        }
    }

    logger::get()->warn("DISCLAIMER: This program is not intended for standalone usage. You need a modified discord "
                        "client that makes use of this!");

    logger::get()->info("Running on port: {}", port);

    httplib::Server server;

    server.set_exception_handler(
        [&](const auto &, auto &, auto exception)
        {
            try
            {
                std::rethrow_exception(exception);
            }
            catch (const std::exception &ex)
            {
                logger::get()->error("Encountered error: {}", ex.what());
            }
            catch (...)
            {
                logger::get()->error("Encountered error: <Unknown>");
            }

            server.stop();
        });

    server.Post("/list",
                [](const auto &req, auto &response)
                {
                    auto props = glz::read_json<std::vector<std::string>>(req.body);
                    auto data  = glz::write_json(patchbay::get().list(props.value_or(std::vector<std::string>{})));

                    response.set_content(data, "application/json");
                });

    server.Post("/link",
                [](const auto &req, auto &response)
                {
                    equicord::link_options parsed;

                    const auto error = glz::read_json(parsed, req.body);

                    if (error)
                    {
                        response.status = 418;
                        return;
                    }

                    patchbay::get().link(std::move(parsed));

                    response.status = 200;
                });

    server.Get("/has-pipewire-pulse",
               [](const auto &, auto &response)
               {
                   auto data = glz::write_json(patchbay::has_pipewire());

                   response.set_content(data, "application/json");
                   response.status = 200;
               });

    server.Get("/unlink",
               [](const auto &, auto &response)
               {
                   patchbay::get().unlink();
                   response.status = 200;
               });

    server.listen("0.0.0.0", port);

    return 0;
}
