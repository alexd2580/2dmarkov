#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "pblt.hpp"
#include "png_image.hpp"

using namespace std;

ostream& log(void) { return cout << "[MAIN] "; }

ostream& err(void) { return cerr << "[MAIN] "; }

using SuccNode = PBLT_node<png_byte>;
using SuccRef = PBLT_ref<png_byte>;

int main(int argc, char* argv[])
{
  /*for(int i = 0; i < 100; i++)
  {
    auto& res = PBLT<uint32_t, PixelInfo>::visit(tree, 42);
    res += 10;
  }

  for(int i = 0; i < 1000; i++)
    cout << tree->find_random((uint32_t)rand()) << endl;*/

  if(argc < 2)
  {
    log() << "Please supply image filename ____.png" << endl;
    return 1;
  }

  string filename(argv[1]);

  PNG_image source(filename + ".png");
  if(!source.load())
    return 1;

  auto height = source.get_height();
  auto width = source.get_width();

  SuccNode tree_root;
  deque<SuccRef> insert_nodes;
  size_t const MAX_SIZE = 5;

  PNG_image destination(PNG_image::next_free_filename(filename), source);
  auto shared_rows = destination.get_data();
  auto raw_rows = shared_rows.get();

  for(size_t y = 0; y < height; y++)
  {
    insert_nodes.clear();
    insert_nodes.push_back(SuccRef(tree_root));

    png_byte* row = raw_rows[y];
    for(size_t x = 0; x < width; x++)
    {
      png_byte color = row[x * 4];
      /*png_byte* ptr = &(row[x * 4]);
      png_byte r = ptr[0];
      png_byte g = ptr[1];
      png_byte b = ptr[2];*/

      size_t size = insert_nodes.size();
      for(size_t i = 0; i < size; i++)
      {
        SuccRef& r = insert_nodes.front();
        insert_nodes.pop_front();
        SuccNode& nxt = r.visit(color & 0b11111000111110001111100011111000);
        insert_nodes.push_back(SuccRef(nxt));
      }
      insert_nodes.push_front(SuccRef(tree_root));
      if(insert_nodes.size() > MAX_SIZE)
        insert_nodes.pop_back();
    }
  }

  SuccRef(tree_root).print();

  // destination.unload();

  log() << "Exit" << endl;
}
