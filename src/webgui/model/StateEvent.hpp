#ifndef __STATEEVENT__
#define __STATEEVENT__
struct StateEvent {
    enum Type {NewClient, OnLine, OffLine, VerifyFail, VerifyOK};
    StateEvent(Type t, int id):type(t), id(id){}

    Type type;
    int id;
};
#endif
