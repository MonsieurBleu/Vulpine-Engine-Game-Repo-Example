#pragma once
#include <../Engine/include/ObjectGroup.hpp>
#include <../Engine/include/Inputs.hpp>
#include <vector>

enum HandItemType
{
    empty,
    lantern
};

class HandItem : public ObjectGroup
{
    private :
        HandItemType type;

    public :
        HandItem(HandItemType type);
        HandItemType getType(){return type;};
};

typedef std::shared_ptr<HandItem> HandItemRef;

class HandItemHandler : public ObjectGroup
{
    private : 
        int currentItem = 0;
        std::vector<HandItemRef> items;

    public :
        HandItemHandler();
        bool inputs(GLFWKeyInfo input);
        virtual void update(bool forceUpdate = false);

        void addItem(HandItemRef newItem);
};


typedef std::shared_ptr<HandItemHandler> HandItemHandlerRef;