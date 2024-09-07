#pragma once
#include <memory>
#include <spdlog/spdlog.h>

namespace equicord
{
    class logger
    {
        struct impl;

      private:
        std::unique_ptr<impl> m_impl;

      private:
        logger();

      public:
        spdlog::logger *operator->() const;

      public:
        [[nodiscard]] static logger &get();
    };
} // namespace equicord
