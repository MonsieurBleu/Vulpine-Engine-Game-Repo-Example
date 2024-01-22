#include <HandItem.hpp>
#include <GameGlobals.hpp>
#include <Globals.hpp>

HandItem::HandItem(HandItemType type) : type(type)
{
    state.hide = ModelStateHideStatus::HIDE;

    switch (type)
    {
    case HandItemType::empty :
        /* code */
        break;
    
    case HandItemType::lantern :
        {
        ModelRef model = newModel(GameGlobals::PBR);
        model->loadFromFolder("ressources/models/lantern/");
        add(model);
        }
        break;
    
    default:
        break;
    }
}

bool HandItemHandler::inputs(GLFWKeyInfo input)
{
    vec2 scroll = globals.mouseScrollOffset();
    if(abs(scroll.y) > 0.f)
    {
        items[currentItem]->state.hide = ModelStateHideStatus::HIDE;

        globals.clearMouseScroll();
        int offset = scroll.y;
        
        int maxId = items.size();
        currentItem = currentItem+offset;

        if(currentItem >= maxId)
            currentItem -= maxId;
        else if(currentItem < 0)
            currentItem += maxId;

        items[currentItem]->state.hide = ModelStateHideStatus::SHOW;
    }   

    return true;
}

HandItemHandler::HandItemHandler()
{
    addItem(HandItemRef(new HandItem(HandItemType::empty)));
}

void HandItemHandler::addItem(HandItemRef newItem)
{
    add(newItem);
    items.push_back(newItem);
}

void HandItemHandler::update(bool forceUpdate){

    

    ObjectGroup::update();
}