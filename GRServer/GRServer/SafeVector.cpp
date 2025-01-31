#include <vector>
#include <shared_mutex>
#include <algorithm>

template<typename T>

class SharedMutexVector 
{
private:
    std::vector<T> vec;
    std::shared_mutex mtx;

public:
    void add(T item) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        vec.push_back(item);
    }

    T get(int index) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return vec.at(index);
    }

    bool remove(T item) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        auto it = std::find(vec.begin(), vec.end(), item);
        if (it != vec.end()) {
            vec.erase(it);
            return true;
        }
        return false;
    }

    std::size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return vec.size();
    }
};