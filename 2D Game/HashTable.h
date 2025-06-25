#pragma once
#include <string>
#include <cstdint>
#include <cstring>
#include <utility>  // For std::pair
#include <typeinfo> // For typeid
#include <SDL3/SDL.h>

template<typename Key, typename Value>
class HashTable
{
public:
    enum EntryState : uint8_t
    {
        EMPTY = 0,
        OCCUPIED = 1,
        DELETED = 2
    };

    struct Entry
    {
        Key key;
        Value value;
        EntryState state;

        Entry() : state(EMPTY) {}
    };

    // Iterator support
    class Iterator
    {
    private:
        Entry* m_table;
        size_t m_capacity;
        size_t m_index;

        // Find next valid entry
        void AdvanceToNext()
        {
            while (m_index < m_capacity && m_table[m_index].state != OCCUPIED)
            {
                m_index++;
            }
        }

    public:
        Iterator(Entry* table, size_t capacity, size_t index)
            : m_table(table), m_capacity(capacity), m_index(index)
        {
            if (m_index < m_capacity && m_table[m_index].state != OCCUPIED)
            {
                AdvanceToNext();
            }
        }

        Iterator& operator++()
        {
            m_index++;
            AdvanceToNext();
            return *this;
        }

        bool operator!=(const Iterator& other) const
        {
            return m_index != other.m_index;
        }

        std::pair<const Key&, Value&> operator*()
        {
            return { m_table[m_index].key, m_table[m_index].value };
        }

        // For range-based for loops
        std::pair<const Key*, Value*> operator->()
        {
            return { &m_table[m_index].key, &m_table[m_index].value };
        }
    };

private:
    Entry* m_table;
    size_t m_capacity;
    size_t m_size;       // Number of occupied entries
    size_t m_tombstones; // Number of deleted entries
    mutable size_t m_modificationCount; // Track modifications for iterator safety

    static constexpr float MAX_LOAD_FACTOR = 0.75f;
    static constexpr size_t MIN_CAPACITY = 16;

    // Hash functions
    size_t HashKey(const std::string& key) const
    {
        // DJB2 algorithm
        size_t hash = 5381;
        for (char c : key)
        {
            hash = ((hash << 5) + hash) + c;
        }
        return hash;
    }

    size_t HashKey(int key) const
    {
        // For integers, mix the bits for better distribution
        size_t x = static_cast<size_t>(key);
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return x;
    }

    // For any other type, just cast to size_t
    template<typename T>
    size_t HashKey(const T& key) const
    {
        return static_cast<size_t>(key);
    }

    // Get the starting index for a key
    size_t GetIndex(const Key& key) const
    {
        return HashKey(key) % m_capacity;
    }

public:
    // Constructor
    explicit HashTable(size_t initialCapacity = MIN_CAPACITY)
        : m_size(0), m_tombstones(0), m_modificationCount(0)
    {
        // Round up to power of 2 for better performance
        m_capacity = MIN_CAPACITY;
        while (m_capacity < initialCapacity)
        {
            m_capacity *= 2;
        }
        m_table = new Entry[m_capacity];
    }

    // Destructor
    ~HashTable()
    {
        delete[] m_table;
    }

    // Disable copy for now
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // Move constructor
    HashTable(HashTable&& other) noexcept
        : m_table(other.m_table),
        m_capacity(other.m_capacity),
        m_size(other.m_size),
        m_tombstones(other.m_tombstones),
        m_modificationCount(other.m_modificationCount)
    {
        other.m_table = nullptr;
        other.m_capacity = 0;
        other.m_size = 0;
        other.m_tombstones = 0;
        other.m_modificationCount = 0;
    }

    // Move assignment
    HashTable& operator=(HashTable&& other) noexcept
    {
        if (this != &other)
        {
            delete[] m_table;

            m_table = other.m_table;
            m_capacity = other.m_capacity;
            m_size = other.m_size;
            m_tombstones = other.m_tombstones;
            m_modificationCount = other.m_modificationCount;

            other.m_table = nullptr;
            other.m_capacity = 0;
            other.m_size = 0;
            other.m_tombstones = 0;
        }
        return *this;
    }

    // Insert or update a key-value pair (with probe limit for safety)
    void Insert(const Key& key, const Value& value)
    {
        // Check if we need to resize
        if ((m_size + m_tombstones) >= m_capacity * MAX_LOAD_FACTOR)
        {
            Resize();
        }

        size_t index = GetIndex(key);
        size_t probeCount = 0;

        // Linear probing with probe limit
        while (m_table[index].state == OCCUPIED && probeCount < m_capacity)
        {
            // if key already exists, update value
            if (m_table[index].key == key)
            {
                m_table[index].value = value;
                return;
            }

            // Move to the next slot
            index = (index + 1) % m_capacity;
            probeCount++;
        }

        // Safety check - this should never happen with proper load factor
        if (probeCount >= m_capacity)
        {
            SDL_Log("HashTable: Critical error - table full!");
            Resize();
            Insert(key, value);  // Retry after resize
            return;
        }

        if (m_table[index].state == DELETED)
        {
            m_tombstones--;
        }

        m_table[index].key = key;
        m_table[index].value = value;
        m_table[index].state = OCCUPIED;
        m_size++;
        m_modificationCount++;  // Track modification
    }

    // Find a value by key (with probe limit)
    Value* Find(const Key& key)
    {
        size_t index = GetIndex(key);
        size_t probeCount = 0;

        // Linear probe with limit
        while (m_table[index].state != EMPTY && probeCount < m_capacity)
        {
            if (m_table[index].state == OCCUPIED && m_table[index].key == key)
            {
                return &m_table[index].value;
            }

            index = (index + 1) % m_capacity;
            probeCount++;
        }

        return nullptr;
    }

    // Const version of find
    const Value* Find(const Key& key) const
    {
        size_t index = GetIndex(key);
        size_t probeCount = 0;

        while (m_table[index].state != EMPTY && probeCount < m_capacity)
        {
            if (m_table[index].state == OCCUPIED && m_table[index].key == key)
            {
                return &m_table[index].value;
            }

            index = (index + 1) % m_capacity;
            probeCount++;
        }

        return nullptr;
    }

    // Operator[] for convenient access
    Value& operator[](const Key& key)
    {
        Value* val = Find(key);
        if (val)
        {
            return *val;
        }

        // Insert default value if not found
        Insert(key, Value{});
        val = Find(key);
        if (!val)
        {
            static Value dummy{};
            return dummy;
        }
        return *val;
    }

    // Remove a key-value pair (with probe limit)
    bool Remove(const Key& key)
    {
        size_t index = GetIndex(key);
        size_t probeCount = 0;

        while (m_table[index].state != EMPTY && probeCount < m_capacity)
        {
            if (m_table[index].state == OCCUPIED && m_table[index].key == key)
            {
                // Mark as deleted (tombstone)
                m_table[index].state = DELETED;
                m_size--;
                m_tombstones++;
                m_modificationCount++;  // Track modification

                // if too many tombstones, resize to clean up
                if (m_tombstones > m_size / 2)
                {
                    Resize();
                }

                return true;
            }
            index = (index + 1) % m_capacity;
            probeCount++;
        }
        return false;
    }

    bool Contains(const Key& key) const
    {
        return Find(key) != nullptr;
    }

    // Get current size
    size_t Size() const { return m_size; }

    // Check if empty
    bool Empty() const { return m_size == 0; }

    // Get capacity
    size_t Capacity() const { return m_capacity; }

    // Get load factor
    float LoadFactor() const
    {
        return m_capacity > 0 ? float(m_size + m_tombstones) / m_capacity : 0.0f;
    }

    // Reserve space for at least n elements
    void Reserve(size_t n)
    {
        if (n > m_capacity * MAX_LOAD_FACTOR)
        {
            size_t newCapacity = MIN_CAPACITY;
            while (newCapacity * MAX_LOAD_FACTOR < n)
            {
                newCapacity *= 2;
            }

            if (newCapacity > m_capacity)
            {
                ResizeToCapacity(newCapacity);
            }
        }
    }

    void Clear()
    {
        for (size_t i = 0; i < m_capacity; i++)
        {
            m_table[i].state = EMPTY;
        }
        m_size = 0;
        m_tombstones = 0;
        m_modificationCount++;  // Track modification
    }

    // Iterator support for range-based for loops
    Iterator begin()
    {
        return Iterator(m_table, m_capacity, 0);
    }

    Iterator end()
    {
        return Iterator(m_table, m_capacity, m_capacity);
    }

    // Debug statistics
    void PrintStats() const
    {
        SDL_Log("=== HashTable Statistics ===");
        SDL_Log("Capacity: %zu", m_capacity);
        SDL_Log("Size: %zu", m_size);
        SDL_Log("Tombstones: %zu", m_tombstones);
        SDL_Log("Load Factor: %.2f%%", LoadFactor() * 100);

        // Calculate average probe distance
        size_t totalProbes = 0;
        size_t maxProbes = 0;

        for (size_t i = 0; i < m_capacity; i++)
        {
            if (m_table[i].state == OCCUPIED)
            {
                size_t idealIndex = GetIndex(m_table[i].key);
                size_t probeDistance = (i >= idealIndex) ?
                    (i - idealIndex) : (m_capacity - idealIndex + i);

                totalProbes += probeDistance;
                if (probeDistance > maxProbes)
                {
                    maxProbes = probeDistance;
                }
            }
        }

        float avgProbes = m_size > 0 ? float(totalProbes) / m_size : 0;
        SDL_Log("Average Probe Distance: %.2f", avgProbes);
        SDL_Log("Max Probe Distance: %zu", maxProbes);
        SDL_Log("===========================");
    }

private:
    // Resize to specific capacity
    void ResizeToCapacity(size_t newCapacity)
    {
        size_t oldCapacity = m_capacity;
        Entry* oldTable = m_table;

        m_capacity = newCapacity;
        m_table = new Entry[m_capacity];
        size_t oldSize = m_size;
        m_size = 0;
        m_tombstones = 0;

        // Rehash all occupied entries
        for (size_t i = 0; i < oldCapacity; i++)
        {
            if (oldTable[i].state == OCCUPIED)
            {
                Insert(oldTable[i].key, oldTable[i].value);
            }
        }

        // Clean up old table
        delete[] oldTable;

        SDL_Log("HashTable resized: %zu -> %zu (occupied: %zu)",
            oldCapacity, m_capacity, m_size);
    }

    // Resize and rehash the table
    void Resize()
    {
        // Double the capacity or shrink if too many tombstones
        size_t newCapacity = (m_size * 2 > m_capacity) ?
            m_capacity * 2 : MIN_CAPACITY;

        while (newCapacity < m_size * 2)
        {
            newCapacity *= 2;
        }

        ResizeToCapacity(newCapacity);
    }
};