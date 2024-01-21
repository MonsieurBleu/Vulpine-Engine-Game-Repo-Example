#pragma once
#include <../Engine/include/ObjectGroup.hpp>
#include "Audio.hpp"


class ModelSound : public ObjectGroup
{
    private :
        AudioSource soundSource;

        
    public :
        virtual void update(bool forceUpdate = false);
};

void ModelSound::update(bool forceUpdate){

    // AudioFile music1;
    // music1.loadOGG("ressources/musics/Endless Space by GeorgeTantchev.ogg");

    // AudioSource musicSource;
    // musicSource
    //     .setBuffer(music1.getHandle())
    //     .setPitch(0)
    //     .play();


    ObjectGroup::update();
}