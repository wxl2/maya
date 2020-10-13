//
// Created by wxl on 2020/10/12.
//

#ifndef MAYA_NOCOPYABLE_H
#define MAYA_NOCOPYABLE_H
namespace maya {
    class nocopyable {
    public:
        nocopyable(const nocopyable &) = delete;

        void operator=(const nocopyable &) = delete;

    protected:
        nocopyable() = default;

        ~nocopyable() = default;
    };
}
#endif //MAYA_NOCOPYABLE_H
