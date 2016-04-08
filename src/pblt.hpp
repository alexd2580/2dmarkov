#include <iostream>
#include <memory>

/**
 * Probabilistic binary tree.
 *
 * Ord Key => typename Key : type of key (efficiently copy-constructible)
 * typename Value : type of value stored at the nodes (should be
 * move-constructible)
 */

template <typename Key, typename Value> class PBLT
{
  using Ptr = std::unique_ptr<PBLT<Key, Value>>;

private:
  uint32_t total_count;

  Ptr left;
  uint32_t left_count;

  Key const key;
  uint32_t this_count;
  Value value;

  Ptr right;
  uint32_t right_count;

public:
  PBLT(Key k) : key(k), value()
  {
    this_count = total_count = 1;
    left_count = right_count = 0;
  }

  virtual ~PBLT(void) = default;

  /**
   * Inserts a new element. If the element already exists then it destroys the
   * value
   */
  static Value& visit(Ptr& p, Key k)
  {
    if(!p)
    {
      p.reset(new PBLT<Key, Value>(k));
      return p->value;
    }
    else
      return p->visit_nonempty(k);
  }

  Value& visit_nonempty(Key k)
  {
    total_count++;
    if(k == key)
    {
      this_count++;
      return value;
    }
    else if(k < key)
    {
      left_count++;
      if(!left)
      {
        left = std::make_unique<PBLT<Key, Value>>(k);
        return left->value;
      }
      else
        return left->visit_nonempty(k);
    }

    right_count++;
    if(!right)
    {
      right = std::make_unique<PBLT<Key, Value>>(k);
      return right->value;
    }
    else
      return right->visit_nonempty(k);
  }

  Value& find_random(uint32_t rnd)
  {
    rnd %= total_count;
    /*std::cout << "rnd " << rnd << " leftc " << left_count << " rightc "
              << right_count << " thisc " << this_count << " totc "
              << total_count << std::endl;*/
    if(rnd < left_count)
      return left->find_random(rnd);
    else if(rnd - left_count < this_count)
      return value;
    else
      return right->find_random(rnd);
  }
};
