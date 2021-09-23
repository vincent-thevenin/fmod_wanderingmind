#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <math.h>

#include "test.h"


#include <sstream>





std::vector<int> kdtree_demo(PointCloud<float>* cloud, const float query[2], const float search_radius)
{
	// construct a kd-tree index:
	typedef nanoflann::KDTreeSingleIndexAdaptor<
		nanoflann::L2_Simple_Adaptor<float, PointCloud<float> > ,
		PointCloud<float>,
		2 /* dim */
		> my_kd_tree_t;

	my_kd_tree_t   index(2 /*dim*/, *cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */) );
	index.buildIndex();

#if 0
	// Test resize of dataset and rebuild of index:
	cloud.pts.resize(cloud.pts.size()*0.5);
	index.buildIndex();
#endif
	// ----------------------------------------------------------------
	// radiusSearch(): Perform a search for the points within search_radius
	// ----------------------------------------------------------------
	{
		std::vector<std::pair<size_t,float> >   ret_matches;

		nanoflann::SearchParams params;
		//params.sorted = false;

		const size_t nMatches = index.radiusSearch(&query[0], search_radius, ret_matches, params);

		// std::cout << "radiusSearch(): radius=" << search_radius << " -> " << nMatches << " matches\n";
		// for (size_t i = 0; i < nMatches; i++)
		// 	std::cout << "idx["<< i << "]=" << ret_matches[i].first << " dist["<< i << "]=" << ret_matches[i].second << std::endl;
		// std::cout << "\n";
        
        std::vector<int> idxes;
        for (size_t i = 0; i < nMatches; i++)
            idxes.push_back(ret_matches[i].first);
        return idxes;
	}

}


FMOD_RESULT F_CALLBACK pcmreadcallback(FMOD_SOUND* /*sound*/, void *data, unsigned int datalen)
{
    static float  t1 = 0, t2 = 0;        // time
    static float  v1 = 0, v2 = 0;        // velocity
    signed short *stereo16bitbuffer = (signed short *)data;

    for (unsigned int count = 0; count < (datalen >> 2); count++)     // >>2 = 16bit stereo (4 bytes per sample)
    {
        *stereo16bitbuffer++ = (signed short)(sin(t1) * 32767.0f);    // left channel
        *stereo16bitbuffer++ = (signed short)(sin(t2) * 32767.0f);    // right channel

        t1 += 0.01f   + v1;
        t2 += 0.0142f + v2;
        v1 += (float)(sin(t1) * 0.002f);
        v2 += (float)(sin(t2) * 0.002f);
    }

    return FMOD_OK;
}

int clean_channels(
    FMOD::Channel** channel,
    int audio_length_ms,
    int buffer_size,
    int chunk_step_ms,
    int* offset,
    std::vector<uint>* playing_queue
) 
{
    //TODO unload from playing queue


    uint chan_pos;
    bool is_playing;
    for(int j=0; j<buffer_size; j++)
    {
        chan_pos = 0;
        channel[j]->getPosition(&chan_pos, FMOD_TIMEUNIT_MS);
        channel[j]->isPlaying(&is_playing);
        if ((static_cast<int>(chan_pos) - (offset[j] + audio_length_ms) > 0 || !is_playing) && channel[j] != nullptr)
        {
            channel[j]->stop();
            (*playing_queue)[j] = -1;
        }
    }

    return 0;
}

int load_sound(
    std::vector<data>* arr, // array of data stuct
    int audio_length_ms, // lengths of audio to play in ms
    FMOD::Channel** channel, // pointer to array of FMOD::Channel
    int chunk_step_ms, // space between chunks in ms
    FMOD_CREATESOUNDEXINFO* exinfo, // array of exinfo
    std::vector<int>* kd_result, // result of kdtree
    int* offset, // offset from start of file in ms to beginning of chunk
    std::vector<uint>* playing_queue, // array of status for streams; -1 for
                                      // not initialized, else id in kdtree
    FMOD::Sound** s, // array of Sound objects
    FMOD::System* system // pointer to FMOD::System object
)
{
    const char* name;
    FMOD_RESULT result;
    bool is_playing;
    int j;
    float progress = 0.0; // progress bar display
    int barWidth = 70; // progress bar display


    for(std::vector<int>::iterator res = kd_result->begin(); res != kd_result->end(); ++res)
    {
        // verify if already playing
        if (
            std::find(
                playing_queue->begin(),
                playing_queue->end(),
                *res
            ) == playing_queue->end()
        ) // true if absent
        //play and add to playing
        {
            //get empty sound
            for (j = 0; j < playing_queue->size(); j++)
            {
                channel[j]->isPlaying(&is_playing);
                if(!is_playing)
                    break; // found open slot for loading sound at j
            }
            (*playing_queue)[j] = *res;

            name = (*arr)[*res].path.data();


            // Sound loading for audio info retrieval
            s[j]->release();
            result = system->createSound(
                name,
                FMOD_CREATECOMPRESSEDSAMPLE,
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
                printf("%s\n", name);
                exit(-1);
            }


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
            std::cout << "ms  : " << length_ms << std::endl;
            std::cout << "byte: " << length_byte << std::endl;


            // Get sound infos
            FMOD_SOUND_TYPE tt;
            FMOD_SOUND_FORMAT ff;
            int num_channels = 0;
            int bits = 0;
            s[j]->getFormat(&tt, &ff, &num_channels, &bits);
            float frequency = 0;
            int priority = 0;
            result = s[j]->getDefaults(&frequency, &priority);
            result = s[j]->release();
            
            // Fill exinfo data
            exinfo[j].cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
            exinfo[j].numchannels       = num_channels;                               /* Number of channels in the sound. */
            exinfo[j].defaultfrequency  = static_cast<int>(frequency);                           /* Default playback rate of sound. */
            offset[j] =  chunk_step_ms * (*arr)[*res].chunk;
            exinfo[j].length            =  static_cast<int>(bits/8 * (*arr)[*res].chunk * chunk_step_ms * frequency / 1000 * num_channels) //offset
                                            + frequency * num_channels * bits/8 * audio_length_ms/1000; //audio length
            exinfo[j].format            = ff;         /* Data format of sound. */
            std::cout << "l   : " << exinfo[j].length << std::endl;
            std::cout << "bits: " << bits << std::endl;
            std::cout << "chan: " << num_channels << std::endl;
            std::cout << "freq: " << frequency << std::endl;
            std::cout << "offs: " << offset[j] << std::endl;
            
            
            // Load sound for playback
            result = system->createSound(
                name,
                FMOD_3D | FMOD_CREATECOMPRESSEDSAMPLE,
                &exinfo[j],
                &s[j]
            );
            if (result != FMOD_OK)
            {
                printf(
                    "FMOD error loading audio! (%d) %s\n",
                    result,
                    FMOD_ErrorString(result)
                );
                printf("%s\n", name);
                exit(-1);
            }

            // Print info
            result = s[j]->getLength(
                &length_ms,
                FMOD_TIMEUNIT_MS
            );
            result = s[j]->getLength(
                &length_byte,
                FMOD_TIMEUNIT_PCMBYTES
            );
            s[j]->getFormat(&tt, &ff, &num_channels, &bits);
            result = s[j]->getDefaults(&frequency, &priority);
            std::cout << "\nms  : " << length_ms << std::endl;
            std::cout << "byte: " << length_byte << std::endl;
            std::cout << "bits: " << bits << std::endl;
            std::cout << "chan: " << num_channels << std::endl;
            std::cout << "freq: " << frequency << std::endl;
            std::cout << "offs: " << offset[j] << std::endl;
            

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
            progress += 1.0/playing_queue->size();


            // Assign sound to FMOD::Channel and set to pause immediately
            result = system->playSound(
                s[j],
                NULL,
                true,
                &(channel[j])
            );
            if (result != FMOD_OK)
            {
                printf(
                    "FMOD error playing sound! (%d) %s\n",
                    result,
                    FMOD_ErrorString(result)
                );
                exit(-1);
            }

            // Change position to offset (i.e. start of chunk)
            result = channel[j]->setPosition(
                chunk_step_ms * (*arr)[*res].chunk,
                FMOD_TIMEUNIT_MS
            );
            if (result != FMOD_OK)
            {
                printf(
                    "FMOD error setting position! (%d) %s\n",
                    result,
                    FMOD_ErrorString(result)
                );
                 exit(-1);
            }

            // Set sound spatial position
            const FMOD_VECTOR soundPos = {
                float{(*arr)[*res].embedding[0]},
                float{(*arr)[*res].embedding[1]},
                0.0
            };
            result = channel[j]->set3DAttributes(
                &soundPos,
                nullptr
            );

            // Unpause sound
            result = channel[j]->setPaused(false);
            if (result != FMOD_OK)
            {
                printf(
                    "FMOD error playing audio! (%d) %s\nid: %d\n",
                    result,
                    FMOD_ErrorString(result),
                    j
                );
                exit(-1);
            }

        }
    }

    return 0;
}



bool getNextLineAndSplitIntoTokens(
    std::ifstream& str,
    float scale,
    PointCloud<float>* cloud,
    std::vector<data>* arr
)
{
    /*
    String processing to transform csv lines into the data structure we need
    Input:
        std::ifstream& str: stream to the csv file with all paths, chunks and
            ids
        float scale: rescale the map data (chunks may to too far apart to
            listen with spatial effects)
        PointCloud<float>* cloud: Output, cloud data to append to for kdtree
            processing
        std::vector<data>* arr: Output, array of type data to append result to

    Output:
        bool: is last line in csv file
    */
    std::vector<std::string>   result;
    std::string                line;
    std::getline(str, line);

    std::stringstream          lineStream(line);
    std::string                cell;

    data d;
    int i = 0;
    float x;
    float y;
    

    while(std::getline(lineStream,cell, ','))
    {
        if (cell == "id")
        {
            return false;
        }
        if (i == 0) // is a path
        {
            d.path = "../mp3/";
            if (cell.front() == '\"')
            {
                d.path.append(cell).append(",");
                std::getline(lineStream,cell, ',');
            }
            d.path.append(cell.substr(0, cell.find("-"))).append(".mp3");
            d.chunk = std::stoi(
                cell.substr(cell.find("-") + 1, std::string::npos)
            );
        }
        else if (i == 1) // is x coordinate
        {
            x = std::stof(cell) * scale;
        }
        else // is y coordinate
        {
            y = std::stof(cell) * scale;
        }
        i++;

    }
    if (cell.empty())
    {
        return true;
    }
    d.embedding = {
        x,
        y
    };
    cloud  ->pts.push_back(
        {
            x,
            y
        }
    );

    arr->push_back(d);

    return false;
}



void load_json(
    float scale,
    std::vector<data>* arr,
    PointCloud<float>* cloud
)
{
    // Load json info.
    std::ifstream ifs("../ocean_samples_fmod.json");
    json jsonfile = json::parse(ifs);
    ifs.close();

    std::string id;
    int j = 0;

    for (auto& element : jsonfile) {
        j++;
        data d;
        for (auto& el : element.items()) {
            if (el.key() == "embedding")
            {
                d.embedding = {
                    static_cast<float>(el.value()[0]) * scale,
                    static_cast<float>(el.value()[1]) * scale
                };
                cloud  ->pts.push_back(
                    {
                        static_cast<float>(el.value()[0]),
                        static_cast<float>(el.value()[1])
                    }
                );
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
        arr->push_back(d);
    }
    int json_size = jsonfile.size();
    jsonfile.clear();
}






int main()
{
    // // Connect to sio client
    // sio::client h;
    // h.connect("http://127.0.0.1:3000");

    // Inits
    FMOD_RESULT result;
    FMOD::System *system = NULL;
    PointCloud<float> cloud;
    const char* name;

    // DEBUG
    // name = "/home/dvic/Downloads/mixkit-male-voice-countdown-917.wav";
    // name = "/mnt/ACA21355A21322FE/WanderingMind/aporee/mp3_trim/Wilde13/01_wilde_13.mp3";
    // name = "../mp3/Wilde13/01_wilde_13.mp3";
    // name = "../mp3/aporee_23439_27224/MKERiverA201401.mp3";
    
    std::vector<data> arr;
    float scale = 1.0; // map rescaling
    float chunk_step_ms = 2.5 * 1000.0;
    int buffer_size = 4095; // [1, 4095]

    result = FMOD::System_Create(&system);      // Create the main system object.
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }

    result = system->init(4095, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.    
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }


    // Load csv info
    std::ifstream ifs("../data_fmod.csv");
    bool done = false;
    // Parse file until done
    while (!done)
    {
        done = getNextLineAndSplitIntoTokens(ifs, 1, &cloud, &arr);
    }


	// Randomize Seed for kdtree
	srand(static_cast<unsigned int>(time(nullptr)));


    FMOD::Channel* channel[buffer_size];
    memset(channel, 0, sizeof(FMOD::Channel*) * buffer_size);
    FMOD::Sound* s [buffer_size];
    float progress = 0.0;
    int barWidth = 70;
    int j = 0;
    int listener = 0;
    int offset[buffer_size];
    memset(offset, 0, sizeof(int) * buffer_size);
    std::vector<uint> playing_queue(buffer_size, -1);
    FMOD_CREATESOUNDEXINFO exinfo[buffer_size];
    memset(exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO) * buffer_size);

    FMOD_VECTOR pos = {0, 0, 0};
    float theta = 0;
    float radius = 1;


    while (true)
    {

        pos.x = std::cos(theta) * radius;
        pos.y = std::sin(theta) * radius;
        theta += 0.1;

        result = system->set3DListenerAttributes(
            listener,
            &pos,
            nullptr,
            nullptr,
            nullptr
        );

        const float query[2] = {pos.x, pos.y};
        const float query_radius = 100;
        std::vector<int> kd_result;
        kd_result = kdtree_demo(&cloud, query, query_radius);
        std::cout << kd_result.size() <<std::endl;

        clean_channels(channel, 5000, buffer_size, chunk_step_ms, offset, &playing_queue);
        load_sound(&arr, 5000, channel, chunk_step_ms, exinfo, &kd_result, offset, &playing_queue, s, system);
    }


    
    
    
    return 0;
}