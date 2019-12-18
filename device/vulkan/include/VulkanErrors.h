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
        explicit VulkanException(const char *message) noexcept
                : message(message) {
        }
        ~VulkanException() noexcept override = default;
        const char *what() const noexcept override {
            return message.c_str();
        }

    private:
        std::string message;
    };

    inline void VulkanCheck(VkResult result) {
        if (result != VK_SUCCESS) {
            throw VulkanException("Operation failed");
        }
    }

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_VULKANERRORS_H