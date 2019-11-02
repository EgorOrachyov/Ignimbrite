//
// Created by Egor Orachyov on 2019-10-26.
//

#ifndef VULKANRENDERER_TESTOBJECTID_H
#define VULKANRENDERER_TESTOBJECTID_H

#include "renderer/ObjectIDBuffer.h"
#include "renderer/FileUtils.h"
#include <string>

struct TestObjectIDBuffer {

    static void test1() {
        ObjectIDBuffer<std::string> strings;

        auto s1id = strings.add("hello");
        auto s2id = strings.add("world");

        std::string &s1 = strings.get(s1id);
        std::string &s2 = strings.get(s2id);

        std::cout << "Message: " << s1 << "," << s2 << "!\n";

        strings.remove(s1id);
        strings.remove(s2id);
    }

    static void test2() {
        try {
            ObjectIDBuffer<std::string> strings;
            auto id = strings.add("some string");
            std::string &s = strings.get(id);
            strings.remove(id);
            strings.remove(id);
        } catch (std::runtime_error& error) {
            std::cout << "Catch error: " << error.what();
        }
    }

    static void test3() {
        const uint32 count = 1000;
        const uint32 remove = 100;

        ObjectID ids[count];
        ObjectIDBuffer<std::string> strings;

        for (uint32 i = 0; i < count; i++) {
            ids[i] = strings.add("");
        }

        for (uint32 i = 0; i < count; i++) {
            strings.get(ids[i]);
        }

        for (uint32 i = 0; i < remove; i++) {
            strings.remove(ids[i]);
        }

        for (uint32 i = 0; i < count; i++) {
            try {
                strings.get(ids[i]);
            } catch (std::runtime_error& error) {
                std::cout << "ID: " << i << " was removed\n";
            }
        }

        for (uint32 i = remove; i < count; i++) {
            strings.remove(ids[i]);
        }

        std::vector<char> data;
        FileUtils::loadData("mu file", data);
    }

    static void run() {
        test1();
        test2();
        test3();
    }

};

#endif //VULKANRENDERER_TESTOBJECTID_H
