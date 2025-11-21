#include <iostream>
#include <string>
#include <array>
#include <memory_resource>

#include "pmr_map_resource.hpp"
#include "pmr_forward_list.hpp"

struct Person {
    std::string name;
    int age;
};

int main() {
    std::cout << "=== PMR Forward List Demo ===\n";

    // Fixed 64 KB buffer
    static std::array<std::byte, 64 * 1024> buffer;

    pmr_map_resource mem(buffer.data(), buffer.size());

    // List of integers
    pmr_forward_list<int> ints(&mem);
    ints.push_front(3);
    ints.push_front(2);
    ints.push_front(1);

    std::cout << "Integers: ";
    for (int v : ints)
        std::cout << v << " ";
    std::cout << "\n";

    // List of custom struct
    pmr_forward_list<Person> people(&mem);
    people.emplace_front("Alice", 22);
    people.emplace_front("Bob", 30);

    std::cout << "People:\n";
    for (const auto& p : people)
        std::cout << "  " << p.name << " (" << p.age << ")\n";

    return 0;
}
