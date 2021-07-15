#include "vendors/json.hpp"
#include <fstream>
#include <iostream>
#include <vector>

using json = nlohmann::json;

struct data
{
    std::vector<float> embedding;
    std::string path;
    int chunk;
};


int main()
{
    std::ifstream i("../ocean_samples_fmod.json");
    // json j;
    // i >> j;
    json j = json::parse(i);
    i.close();
    std::vector<data> arr;
    std::string id;
    
    // iterate the array
    // for (json::iterator it = j.begin(); it != j.end(); ++it) {
    //     // std::cout << *it << '\n';
    //     std::cout << it.key() << " : " << it.value() << "\n";
    //     // std::string ss = it->get<std::string>();
    //     // arr.push_back(ss);

    //     // range-based for
    //     for (auto& element : *it) {
    //         std::cout << element << '\n';
    //     }
    // }

    for (auto& element : j) {
        std::cout << element << '\n';
        // for (auto& elementt : element) {
        //     std::cout << elementt << '\n';
        // }
        data d;
        for (auto& el : element.items()) {
            std::cout << el.key() << " : " << el.value() << "\n";
            if (el.key() == "embedding")
            {
                d.embedding = {el.value()[0], el.value()[1]};
            }
            else
            {
                id = el.value();
                d.path = id.substr(0, id.find("-"));
                d.chunk = std::stoi(
                    id.substr(id.find("-") + 1, std::string::npos)
                );
            }
        }
        arr.push_back(d);
    }


    std::cout << j.size() << std::endl;

    return EXIT_SUCCESS;
}
