#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <memory>

///
/// シングルトンパターンの基底クラス
///
template <class T>
class Singleton
{
public:
    static T& instance() {
        static pointer_type instance(createInstance());

        return getReference(instance);
    }

protected:
    Singleton() {
    }

private:
    typedef std::auto_ptr<T> pointer_type;

    static T* createInstance() { return new T(); }
    static T& getReference(const pointer_type &p) { return *p; }

    explicit Singleton(const Singleton&);
    Singleton& operator=(const Singleton&);
};

#endif // SINGLETON_HPP
