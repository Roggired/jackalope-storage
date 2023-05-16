//
// Created by ego on 16.05.23.
//

#include "memory_map_factory.hpp"

using namespace memory;


[[maybe_unused]]
MemoryMap* memory::initLocalMemoryMap(MemoryMapsConfig config) {
    return new MemoryMap(config.localMemSize);
}

[[maybe_unused]]
void memory::destroyLocalMemoryMap(MemoryMap* localMemoryMap) {
    delete localMemoryMap;
}

[[maybe_unused]]
[[nodiscard]]
MemoryMap* memory::initSharedMemoryMap(MemoryMapsConfig config) {
    std::lock_guard<std::mutex> locker(sharedMemoryMapMutex);
    //TODO
    // Need to think about allocation underlying STL unordered_map of MemoryMap in the shared memory
    // seems like there is a problem with that, as soon as we need to provide allocator for STL collection.
    // Questions:
    // 1. Can we use a ready allocator and just customize the way an allocator gets ptr to the memory space?
    // 2. Can we use Boost shared mem? Is it ready by-design?
    // 3. Can we reuse semnet/shared_memory.hpp?
    // ---
    // Seems like for the diploma I can use only LocalMemoryMap int he multithreading way and implement SharedMemoryMap later
    return nullptr;
}

[[maybe_unused]]
void memory::detachFromSharedMemoryMap(MemoryMap* sharedMemoryMap) {
    //TODO
}

[[maybe_unused]]
[[nodiscard]]
MemoryMap* memory::getSharedMemoryMap() {
    return sharedMemoryMapPtr;
}
