#ifndef SHARED_H
#define SHARED_H

#include <utility>
#include <mutex>

namespace cs540{

    std::mutex mtx;

    class ControlBlockBase{
        int count;
    public:
        ControlBlockBase() : count(1) {}
        virtual ~ControlBlockBase() {}
        void incr(){
            std::unique_lock<std::mutex> lock(mtx);
            count++;
        }
        void decr(){
            std::unique_lock<std::mutex> lock(mtx);
            if (--count == 0) delete this;
        }

        int getCount(){
            std::unique_lock<std::mutex> lock(mtx);
            return count;
        }
    };

    template<class U>
    class ControlBlock : public ControlBlockBase{
        U *ctrl_ptr; //managed ptr
    public:
        ControlBlock(U *p) : ctrl_ptr(p) {}
        ~ControlBlock() { delete ctrl_ptr; }
    };

    template <typename T>
    class SharedPtr {
    public:
        T *ptr; //stored ptr
        ControlBlockBase *block;

        //Default Constructor
        SharedPtr() : ptr(nullptr), block(nullptr) {}

        //Template Constructor
        template<typename U>
        explicit SharedPtr(U *obj){
            // Constructs a smart pointer that points to the given object.
            //The reference count is initialized to one.
            ptr = obj;
            block = new ControlBlock<U>(obj);
        }

        //Copy Constructor
        SharedPtr(const SharedPtr &p) :  ptr(p.ptr), block(p.block){
            //If p is not null, then reference count of the managed object is incremented.
            if (block) block->incr();
        }

        //Template Copy Constructor
        template <typename U>
        SharedPtr(const SharedPtr<U> &p) : ptr(static_cast<T *>(p.ptr)), block(p.block){
            //If U * is not implicitly convertible to T *, use will result in a compile-time
            //error when the compiler attempts to instantiate the member template.
            if (block) block->incr();
        }

        template <typename U>
        friend void swap(SharedPtr &lhs, SharedPtr<U> &rhs){
            using std::swap;

            T *blah = rhs.ptr;
            swap(lhs.ptr, blah);
            swap(lhs.block, rhs.block);
        }

        //Copy Assignment
        SharedPtr &operator=(SharedPtr p){
            // if (*this != p) {
            //     if (block) block->decr();
            //     ptr = p.ptr;
            //     block = p.block;
            //     if (block) block->incr();
            // }
            swap(*this, p);
            return *this;
        }

        //Template Copy Assignment
        template <typename U>
        SharedPtr<T> &operator=(SharedPtr<U> p){
            // if (*this != p) {
            //     if (block) block->decr();
            //     ptr = p.ptr;
            //     block = p.block;
            //     if (block) block->incr();
            // }
            swap(*this, p);
            return *this;
        }

        //Destructor
        ~SharedPtr(){
            if (block) block->decr();
        }

        //Reset
        void reset(){
            //The smart pointer is set to point to the null pointer.
            //The reference count for the currently pointed to object, if any, is decremented.
            if (block) block->decr();
            ptr = nullptr;
            block = nullptr;
        }

        template <typename U>
        void reset(U *p){
            //Replace owned resource with another pointer. If the owned resource has no other references,
            //it is deleted. If p has been associated with some other smart pointer, UB
            reset();
            ptr = p;
            block = new ControlBlock<U>(p);
        }

        //Getter for Pointer
        T *get() const{
            return ptr;
        }

        //Overload * Operator
        T &operator*() const{
            return *ptr;
        }

        //Overload -> operator
        T *operator->() const{
            return ptr;
        }

        //Returns true if has stored pointer
        explicit operator bool() const{
            return get() != nullptr;
        }

    };

    template <typename T1, typename T2>
    bool operator==(const SharedPtr<T1> &a, const SharedPtr<T2> &b){
        return a.get() == b.get();
    }

    template <typename T>
    bool operator==(const SharedPtr<T> &a, std::nullptr_t b){
        return a.get() == nullptr;
    }
    template <typename T>
    bool operator==(std::nullptr_t a, const SharedPtr<T> &b){
        return nullptr == b.get();
    }

    template <typename T1, typename T2>
    bool operator!=(const SharedPtr<T1> &a, const SharedPtr<T2> &b){
        return a.get() != b.get();
    }

    template <typename T>
    bool operator!=(const SharedPtr<T> &a, std::nullptr_t b){
        return a.get() != nullptr;
    }
    template <typename T>
    bool operator!=(std::nullptr_t a, const SharedPtr<T> &b){
        return nullptr != b.get();
    }

    template <typename T, typename U>
    SharedPtr<T> static_pointer_cast(const SharedPtr<U> &sp){
        SharedPtr<T> casted = SharedPtr<T>(sp);
        casted.ptr = static_cast<T *>(sp.get());
        return casted;
    }

    template <typename T, typename U>
    SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U> &sp){
        SharedPtr<T> casted = SharedPtr<T>(sp);
        casted.ptr = dynamic_cast<T *>(sp.get());
        return casted;
    }

}

#endif
