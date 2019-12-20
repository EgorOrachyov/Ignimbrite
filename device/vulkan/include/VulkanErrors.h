//
// Created by Egor Orachyov on 2019-11-07.
//

#ifndef IGNIMBRITELIBRARY_VULKANERRORS_H
#define IGNIMBRITELIBRARY_VULKANERRORS_H

#include <exception>
#include <string>

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

#endif //IGNIMBRITELIBRARY_VULKANERRORS_H