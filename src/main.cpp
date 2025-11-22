#include <iostream>
#include <memory_resource>

#include "FixedBlockResource.h"
#include "ForwardList.h"

struct Person {
    std::string name;
    int age;
};

int main() {

    FixedBlockResource mem(4096);

    // ForwardList<int>
    std::cout << "[ForwardList<int>]\n";

    ForwardList<int> list(&mem);

    list.push_back(10);
    list.push_back(20);
    list.push_back(30);

    std::cout << "Values: ";
    for (int v : list)
        std::cout << v << " ";
    std::cout << "\n";

    std::cout << "Manual iteration: ";
    for (auto it = list.begin(); it != list.end(); ++it)
        std::cout << *it << " ";
    std::cout << "\n\n";

    // Memory reuse demo
    {
        std::cout << "[Temporary list for reuse test]\n";
        ForwardList<int> temp(&mem);
        temp.push_back(111);
        temp.push_back(222);

        std::cout << "Temp: ";
        for (int x : temp)
            std::cout << x << " ";
        std::cout << "\n";

        std::cout << "Destroying temp...\n\n";
    }

    std::cout << "[New list after destruction of temp]\n";
    ForwardList<int> reused(&mem);
    reused.push_back(999);
    reused.push_back(888);

    std::cout << "Reused: ";
    for (int x : reused)
        std::cout << x << " ";
    std::cout << "\n\n";

    // ForwardList<Person>
    std::cout << "[ForwardList<Person>]\n";
    ForwardList<Person> people(&mem);

    people.push_back({"Alice", 30});
    people.push_back({"Bob", 25});
    people.push_back({"Charlie", 40});

    for (const auto& p : people)
        std::cout << p.name << " (" << p.age << ")\n";

    std::cout << "\n";

    return 0;
}
