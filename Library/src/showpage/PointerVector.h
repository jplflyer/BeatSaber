#ifndef SRC_LIB_POINTERVECTOR_H_
#define SRC_LIB_POINTERVECTOR_H_

#include <vector>
#include <functional>

#include "JSON_Serializable.h"

/**
 * This is the base class for any vector that holds pointers to data we keep.
 * On destruction or a call to erase, we destroy not only our contents, but the
 * pointed-to values. If you pull something from us, the person pulling it owns it.
 *
 * It is possible to abuse this object by doing things like calling clear() directly.
 * Please don't.
 */
template <class ObjectType>
class PointerVector : public std::vector<ObjectType *> {
private:
    /** If true, then on destruction or erase, we release the data we point to. */
	bool iOwnTheData;

public:
    /** Constructor. */
	PointerVector(): iOwnTheData(true) {}

	/**
	 * THe default behavior is the vector owns the data, but you can override that
	 * using this constructor. Note that this ONLY effects the clear() and eraseAll()
	 * methods plus the destructor.
	 */
	PointerVector(bool owns): iOwnTheData(owns) {}

	/**
	 * Destructor does an eraseAll().
	 */
	virtual ~PointerVector() { eraseAll(); }

    /**
     * If this vector owns the data, then we'll delete it on erase/destruction.
     */
	bool ownsTheData() const { return iOwnTheData; }

	/**
	 * Do we have this object?
	 */
	bool contains(ObjectType *ptr) {
		for (auto iter = this->begin(); iter != this->end(); ++iter) {
			if (*iter == ptr) {
				return true;
			}
		}
		return false;
	}

    /**
     * What is the index of this pointer?
     *
     * @returns Index of this pointer or -1 if not found.
     */
	int indexOf(ObjectType *ptr) {
		int retVal = 0;
		for (auto iter = this->begin(); iter != this->end(); ++iter) {
			if (*iter == ptr) {
				return retVal;
			}
			++retVal;
		}
		return -1;
	}

	/**
	 * This is an attempt to catch people who call clear() instead of eraseAll(),
	 * but if they're passing us around as a superclass, they can cheat, as the STL
	 * people, in their wisdom, don't make their methods virtual.
	 */
	void clear() {
		eraseAll();
	}

	/**
	 * Erase our contents, freeing any memory we're pointed to.
	 */
	void eraseAll() {
		if (iOwnTheData) {
			for (ObjectType *value: *this) {
				delete value;
			}
		}

		std::vector<ObjectType *>::clear();
	}

	/**
	 * This removes this object without deleting the object. The caller now owns the data.
	 */
	void remove(ObjectType *ptr) {
		for (auto iter = this->begin(); iter != this->end(); ++iter) {
			if (*iter == ptr) {
				this->erase(iter);
				break;
			}
		}
	}

    /**
     * perform a binary search. the compare function should return < 0 if we need
     * to move closer to the front == 0 for a matchin, and > 0 to move further ahead.
     */
    int binarySearchForIndex(std::function<bool(ObjectType *)> compare, bool returnInsertLoc = false, int low = 0, int high = 0) const {
        if (high == 0) {
            high = this->size();
        }
        if (low >= high) {
            return returnInsertLoc ? low : -1;
        }
        int mid = (low + high) / 2;
        ObjectType *ptr = this->at(mid);
        int rv = compare(ptr);

        if (rv < 0) {
            return binarySearchForIndex(compare, returnInsertLoc, low, mid);
        }
        if (rv > 0) {
            return binarySearchForIndex(compare, returnInsertLoc, mid + 1, high);
        }

        return mid;
    }
};

template <class ObjectType>
class JSON_Serializable_PointerVector : public PointerVector<ObjectType> {
public:
    JSON_Serializable_PointerVector() { }
    JSON_Serializable_PointerVector(bool v): PointerVector<ObjectType>(v) { }

    void fromJSON(const nlohmann::json & array) {
        for (auto iter = array.begin(); iter != array.end(); ++iter) {
            nlohmann::json obj = *iter;

            ObjectType * thisDiff = new ObjectType();
            thisDiff->fromJSON(obj);
            this->push_back(thisDiff);
        }
    }

    void toJSON(nlohmann::json & array) const {
        for (ObjectType * obj: *this) {
            nlohmann::json childJson = nlohmann::json::object();
            obj->toJSON(childJson);
            array.push_back(childJson);
        }
    }
};

#endif /* SRC_LIB_POINTERVECTOR_H_ */
