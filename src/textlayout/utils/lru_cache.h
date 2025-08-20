/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_TEXTLAYOUT_UTILS_LRU_CACHE_H_
#define SRC_TEXTLAYOUT_UTILS_LRU_CACHE_H_

#include <functional>
#include <memory>
#include <unordered_set>
#include <utility>

#include "src/textlayout/utils/type_helper.h"  // hash_t

namespace android {

/**
 * GenerationCache callback used when an item is removed
 */
template <typename EntryKey, typename EntryValue>
class OnEntryRemoved {
 public:
  virtual ~OnEntryRemoved() {}
  virtual void operator()(const EntryKey& key, const EntryValue& value) = 0;
};  // class OnEntryRemoved

template <typename TKey, typename TValue>
class LruCache {
 public:
  explicit LruCache(uint32_t maxCapacity);
  virtual ~LruCache();

  enum Capacity {
    kUnlimitedCapacity,
    kDefaultCapacity = 10'000,
  };

  void setOnEntryRemovedListener(OnEntryRemoved<TKey, TValue>* listener);
  size_t size() const;
  TValue* get(const TKey& key);
  TValue* put(const TKey& key, TValue value);
  bool remove(const TKey& key);
  bool removeOldest();
  void clear();
  TValue* peekOldestValue();

 private:
  LruCache(const LruCache& that);  // disallow copy constructor

  // Super class so that we can have entries having only a key reference, for
  // searches.
  class KeyedEntry {
   public:
    virtual const TKey& getKey() const = 0;
    // Make sure the right destructor is executed so that keys and values are
    // deleted.
    virtual ~KeyedEntry() {}

    bool operator==(const KeyedEntry& rhs) const {
      return this->getKey() == rhs.getKey();
    }
    bool operator==(const KeyedEntry* rhs) const {
      return this->getKey() == rhs->getKey();
    }
  };

  class Entry final : public KeyedEntry {
   public:
    TKey key;
    TValue value;
    Entry* parent;
    Entry* child;

    Entry(const TKey& _key, TValue&& _value)
        : key(_key),
          value(std::move(_value)),
          parent(nullptr),
          child(nullptr) {}
    const TKey& getKey() const final { return key; }
  };

  class EntryForSearch : public KeyedEntry {
   public:
    const TKey& key;
    explicit EntryForSearch(const TKey& key_) : key(key_) {}
    const TKey& getKey() const final { return key; }
  };

  struct HashForEntry : public std::hash<KeyedEntry*> {
    size_t operator()(const KeyedEntry* entry) const {
      return hash_type(entry->getKey());
    }
  };

  struct EqForEntry : public std::equal_to<KeyedEntry*> {
    bool operator()(const KeyedEntry* a, const KeyedEntry* b) const {
      return a->getKey() == b->getKey();
    }
  };

  // All entries in the set will be Entry*. Using the weaker KeyedEntry as to
  // allow entries that have only a key reference, for searching.
  typedef std::unordered_set<KeyedEntry*, HashForEntry, EqForEntry> LruCacheSet;

  void attachToCache(Entry* entry);
  void detachFromCache(Entry* entry);

  typename LruCacheSet::iterator findByKey(const TKey& key) {
    EntryForSearch entryForSearch(key);
    typename LruCacheSet::iterator result = mSet->find(&entryForSearch);
    return result;
  }

  std::unique_ptr<LruCacheSet> mSet;
  OnEntryRemoved<TKey, TValue>* mListener;
  Entry* mOldest;
  Entry* mYoungest;
  uint32_t mMaxCapacity;
  //    TValue mNullValue;

 public:
  // To be used like:
  // while (it.next()) {
  //   it.value(); it.key();
  // }
  class Iterator {
   public:
    explicit Iterator(const LruCache<TKey, TValue>& cache)
        : mCache(cache),
          mIterator(mCache.mSet->begin()),
          mBeginReturned(false) {}

    bool next() {
      if (mIterator == mCache.mSet->end()) {
        return false;
      }
      if (!mBeginReturned) {
        // mIterator has been initialized to the beginning and
        // hasn't been returned. Do not advance:
        mBeginReturned = true;
      } else {
        std::advance(mIterator, 1);
      }
      bool ret = (mIterator != mCache.mSet->end());
      return ret;
    }

    const TValue& value() const {
      // All the elements in the set are of type Entry. See comment in the
      // definition of LruCacheSet above.
      return reinterpret_cast<Entry*>(*mIterator)->value;
    }

    const TKey& key() const { return (*mIterator)->getKey(); }

   private:
    const LruCache<TKey, TValue>& mCache;
    typename LruCacheSet::iterator mIterator;
    bool mBeginReturned;
  };
};

// Implementation is here, because it's fully templated
template <typename TKey, typename TValue>
LruCache<TKey, TValue>::LruCache(uint32_t maxCapacity)
    : mSet(new LruCacheSet()),
      mListener(nullptr),
      mOldest(nullptr),
      mYoungest(nullptr),
      mMaxCapacity(maxCapacity) {
  mSet->max_load_factor(1.0);
}

template <typename TKey, typename TValue>
LruCache<TKey, TValue>::~LruCache() {
  // Need to delete created entries.
  clear();
}

template <typename K, typename V>
void LruCache<K, V>::setOnEntryRemovedListener(OnEntryRemoved<K, V>* listener) {
  mListener = listener;
}

template <typename TKey, typename TValue>
size_t LruCache<TKey, TValue>::size() const {
  return mSet->size();
}

template <typename TKey, typename TValue>
TValue* LruCache<TKey, TValue>::get(const TKey& key) {
  typename LruCacheSet::const_iterator find_result = findByKey(key);
  if (find_result == mSet->end()) {
    return nullptr;
  }
  // All the elements in the set are of type Entry. See comment in the
  // definition of LruCacheSet above.
  Entry* entry = reinterpret_cast<Entry*>(*find_result);
  detachFromCache(entry);
  attachToCache(entry);
  return &(entry->value);
}

template <typename TKey, typename TValue>
TValue* LruCache<TKey, TValue>::put(const TKey& key, TValue value) {
  if (mMaxCapacity != kUnlimitedCapacity && size() >= mMaxCapacity) {
    removeOldest();
  }

  if (findByKey(key) != mSet->end()) {
    return nullptr;
  }

  Entry* newEntry = new Entry(key, std::move(value));
  mSet->insert(newEntry);
  attachToCache(newEntry);
  return &(newEntry->value);
}

template <typename TKey, typename TValue>
bool LruCache<TKey, TValue>::remove(const TKey& key) {
  typename LruCacheSet::const_iterator find_result = findByKey(key);
  if (find_result == mSet->end()) {
    return false;
  }
  // All the elements in the set are of type Entry. See comment in the
  // definition of LruCacheSet above.
  Entry* entry = reinterpret_cast<Entry*>(*find_result);
  mSet->erase(entry);
  if (mListener) {
    (*mListener)(entry->key, entry->value);
  }
  detachFromCache(entry);
  delete entry;
  return true;
}

template <typename TKey, typename TValue>
bool LruCache<TKey, TValue>::removeOldest() {
  if (mOldest != nullptr) {
    return remove(mOldest->key);
    // TODO(hfuttyh): should probably abort if false
  }
  return false;
}

template <typename TKey, typename TValue>
TValue* LruCache<TKey, TValue>::peekOldestValue() {
  if (mOldest) {
    return &(mOldest->value);
  }
  return nullptr;
}

template <typename TKey, typename TValue>
void LruCache<TKey, TValue>::clear() {
  if (mListener) {
    for (Entry* p = mOldest; p != nullptr; p = p->child) {
      (*mListener)(p->key, p->value);
    }
  }
  mYoungest = nullptr;
  mOldest = nullptr;
  for (auto entry : *mSet.get()) {
    delete entry;
  }
  mSet->clear();
}

template <typename TKey, typename TValue>
void LruCache<TKey, TValue>::attachToCache(Entry* entry) {
  if (mYoungest == nullptr) {
    mYoungest = mOldest = entry;
  } else {
    entry->parent = mYoungest;
    mYoungest->child = entry;
    mYoungest = entry;
  }
}

template <typename TKey, typename TValue>
void LruCache<TKey, TValue>::detachFromCache(Entry* entry) {
  if (entry->parent != nullptr) {
    entry->parent->child = entry->child;
  } else {
    mOldest = entry->child;
  }
  if (entry->child != nullptr) {
    entry->child->parent = entry->parent;
  } else {
    mYoungest = entry->parent;
  }

  entry->parent = nullptr;
  entry->child = nullptr;
}

}  // namespace android
#endif  // SRC_TEXTLAYOUT_UTILS_LRU_CACHE_H_
