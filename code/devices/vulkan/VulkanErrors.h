/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_VULKANERRORS_H
#define IGNIMBRITE_VULKANERRORS_H

#include <exception>
#include <cstring>

namespace ignimbrite {

    class InvalidEnum : public std::exception {
    public:
        ~InvalidEnum() noexcept override = default;
        const char *what() const noexcept override {
            return "VulkanDefinitions: invalid input enum";
        }
    };

    class VulkanException : public std::exception {
    public:
        explicit VulkanException(const char *message) noexcept {
            std::strncat(buffer, message, MESSAGE_SIZE);
        }
        ~VulkanException() noexcept override = default;
        const char *what() const noexcept override {
            return buffer;
        }
    private:
        static const uint32 MESSAGE_SIZE = 256;
        char buffer[sizeof(char) * MESSAGE_SIZE] = { '\0' };
    };

} // namespace ignimbrite

#define VK_RESULT_ASSERT(result, message)                               \
    do {                                                                \
        if ((result) != VK_SUCCESS)                                     \
            throw VulkanException(message);                             \
    } while (false);

#define VK_TRUE_ASSERT(condition, message)                              \
    {                                                                   \
        if (!(condition)) {                                             \
            throw VulkanException(message);                             \
        }                                                               \
    }

#endif //IGNIMBRITE_VULKANERRORS_H