typedef struct Trigger Trigger;
struct Trigger
{
    TriggerKind kind;
    Input input;
    Shape shape;
    ItemKind collect_item_kind;
    Item *collect_item;
    EntityHandle collideEntity;
    f32 secondsToAlarm;
};

typedef struct Entity Entity;
struct Entity
{
    u64 type;
    u32 generation;
    u32 id;
    
    Entity* parent;
    Entity* firstChild;
    Entity* lastChild;
    
    v2 pos;
    f32 dP;
    f32 ddP;
    v2 size;
    f32 rotation;
    
    Sprite sprite;
    Rect2D rect;
    f32 tint;
    
    Trigger trigger;
};

Entity* CreateActor(v2 pos, u32 sprite, u32 type)
{
}