#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>

// #include "api/core/inc/fmod.hpp"
// #include "api/core/inc/fmod.h"
// #include "api/core/inc/fmod_errors.h"

#include "test.h"

int main()
{
    FMOD_RESULT result;
    FMOD::System *system = NULL;

    FMOD_CREATESOUNDEXINFO sexinf;
    sexinf.cbsize = sizeof(sexinf);
    sexinf.fileoffset = 0;

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

    const char* name = "/home/dvic/Downloads/mixkit-male-voice-countdown-917.wav";
    // const char* name = "/mnt/ACA21355A21322FE/WanderingMind/mp3/Wilde13/01_wilde_13.mp3";

    FMOD::Channel* channel;

    FMOD::Sound *s;
    result = system->createSound(
        name,
        FMOD_OPENONLY,
        NULL,
        &s
    );
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }

    uint length_ms;
    uint length_byte;
    result = s->getLength(
        &length_ms,
        FMOD_TIMEUNIT_MS
    );
    result = s->getLength(
        &length_byte,
        FMOD_TIMEUNIT_PCMBYTES
    );

    std::cout << "length ms: " << length_ms << std::endl;
    std::cout << "length byte: " << length_byte << std::endl;

    // result =  s->seekData(
    //     0
    // );
    // if (result != FMOD_OK)
    // {
    //     printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    //     exit(-1);
    // }

    // const uint BUF_SIZE = length_byte/2;
    // uint buf[BUF_SIZE];
    // uint read = 0;
    // result = s->readData(
    //     buf,
    //     BUF_SIZE,
    //     &read
    // );
    // std::cout << "read: " << read << std::endl;
    // if (result != FMOD_OK)
    // {
    //     printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    //     exit(-1);
    // }

    // FMOD::Sound *s2;
    // FMOD_CREATESOUNDEXINFO sexinf;
    // sexinf.cbsize = sizeof(sexinf);
    // sexinf.length = BUF_SIZE;

    // const char* name2 = (const char *)&buf;

    // result = system->createSound(
    //     name2,
    //     FMOD_OPENMEMORY,
    //     &sexinf,
    //     &s2
    // );
    // if (result != FMOD_OK)
    // {
    //     printf("FMOD error s2! (%d) %s\n", result, FMOD_ErrorString(result));
    //     exit(-1);
    // }


    // free(s);
    FMOD::Sound *s2;


    result = system->createSound(
        name,
        FMOD_CREATECOMPRESSEDSAMPLE,
        NULL,
        &s2
    );
    if (result != FMOD_OK)
    {
        printf("FMOD error create! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
    

    system->playSound(
        s2,
        NULL,
        false,
        &channel
    );

    sleep(1);

    

    sleep(15);
}
