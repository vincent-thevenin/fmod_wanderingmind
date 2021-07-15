#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <math.h>

#include "test.h"


int main()
{
    FMOD_RESULT result;
    FMOD::System *system = NULL;

    result = FMOD::System_Create(&system);      // Create the main system object.
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }

    result = system->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }

    // const char* name = "/home/dvic/Downloads/mixkit-male-voice-countdown-917.wav";
    // const char* name = "/mnt/ACA21355A21322FE/WanderingMind/aporee/mp3_trim/Wilde13/01_wilde_13.mp3";
    // const char* name = "../mp3/Wilde13/01_wilde_13.mp3";
    const char* name;
    std::vector<data> arr;
    std::string id;
    float scale = 10.0;
    float chunk_step_ms = 2.5 * 1000.0;


    // Load json info.
    std::ifstream ifs("../ocean_samples_fmod.json");
    json jsonfile = json::parse(ifs);
    ifs.close();

    for (auto& element : jsonfile) {
        data d;
        for (auto& el : element.items()) {
            if (el.key() == "embedding")
            {
                d.embedding = {
                    static_cast<float>(el.value()[0]) * scale,
                    static_cast<float>(el.value()[1]) * scale
                };
            }
            else
            {
                id = el.value();
                d.path = "../mp3/";
                d.path.append(id.substr(0, id.find("-"))).append(".mp3");
                d.chunk = std::stoi(
                    id.substr(id.find("-") + 1, std::string::npos)
                );
            }
        }
        arr.push_back(d);
    }
    int json_size = jsonfile.size();
    jsonfile.clear();


    FMOD::Channel* channel [json_size];
    FMOD::Sound* s [json_size];
    float progress = 0.0;
    int barWidth = 70;
    int j = 0;
    int listener = 0;
    for(std::vector<data>::iterator it = arr.begin(); it != arr.end(); ++it)
    {
    
        name = it->path.data();
        result = system->createSound(
            name,
            FMOD_CREATECOMPRESSEDSAMPLE | FMOD_3D,
            NULL,
            &s[j]
        );
        if (result != FMOD_OK)
        {
            printf(
                "FMOD error loading audio! (%d) %s\n",
                result,
                FMOD_ErrorString(result)
            );
            printf("%s", name);
            exit(-1);
        }

        // Display progress.
        std::cout << "[";
        int pbar_pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i)
        {
            if (i < pbar_pos) std::cout << "=";
            else if (i == pbar_pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();
        progress += 1.0/json_size;

        // Get lengths
        uint length_ms;
        uint length_byte;
        result = s[j]->getLength(
            &length_ms,
            FMOD_TIMEUNIT_MS
        );
        result = s[j]->getLength(
            &length_byte,
            FMOD_TIMEUNIT_PCMBYTES
        );
        // std::cout << length_ms << std::endl;
        // std::cout << length_byte << std::endl;
        
        // Play sound
        result = system->playSound(
            s[j],
            NULL,
            true,
            &(channel[j])
        );
        // if (result != FMOD_OK)
        // {
        //     printf(
        //         "FMOD error playing sound! (%d) %s\n",
        //         result,
        //         FMOD_ErrorString(result)
        //     );
        //     int* a[1000];
        //     exit(-1);
        // }

        result = channel[j]->setPosition(
            chunk_step_ms * it->chunk,
            FMOD_TIMEUNIT_MS
        );
        const FMOD_VECTOR soundPos = {
            float{it->embedding[0]},
            float{it->embedding[1]},
            0.0
        };
        result = channel[j]->set3DAttributes(
            &soundPos,
            nullptr
        );

        j++;
    }

    FMOD_VECTOR pos = {0, 0, 0};

    result = system->set3DListenerAttributes(
        listener,
        &pos,
        nullptr,
        nullptr,
        nullptr
    );
    std::cout << std::endl;


    for (j=0; j<json_size; j++)
    {
        result = channel[j]->setPaused(false);
        // if (result != FMOD_OK)
        // {
        //     printf(
        //         "FMOD error playing audio! (%d) %s\nid: %d\n",
        //         result,
        //         FMOD_ErrorString(result),
        //         j
        //     );
        //     exit(-1);
        // }
    }

    bool is_playing;
    bool is_virtual;
    int n_virtual;
    int n_playing;
    float theta = 0;
    float radius = 1;
    do
    {
        n_playing = 0;
        n_virtual = 0;
        for (j=0; j<json_size; j++)
        {
            channel[j]->isVirtual(&is_virtual);
            n_virtual += static_cast<int>(is_virtual);
        }
        for (j=0; j<json_size; j++)
        {
            channel[j]->isPlaying(&is_playing);
            n_playing += static_cast<int>(is_playing);
        }
        for (j=0; j<json_size; j++)
        {
            channel[j]->isPlaying(&is_playing);
            if (is_playing)
                break;
        }

        pos.x = std::cos(theta) * radius;
        pos.y = std::sin(theta) * radius;
        theta += 0.002;
        result = system->set3DListenerAttributes
        (
            listener,
            &pos,
            nullptr,
            nullptr,
            nullptr
        );
        system->update();
        usleep((int)(1.0/30 * 1000000));
        std::cout <<
        "x: " << pos.x <<
        ", y: " << pos.y <<
        ", num virt: " << static_cast<float>(n_virtual)/n_playing*100 << "%" <<
        ", num playing: " << static_cast<float>(n_playing)/json_size*100 << "%" <<
        " \r";
        std::cout.flush();
    } while (is_playing);
    std::cout << std::endl;
    

    // Clean up.
    for (j=0; j<json_size; j++)
    {
        s[j]->release();
    }
    system->release();
    return 0;
}
