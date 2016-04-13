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

using Succ_node = PBLT_node<uint32_t>;
using Succ_ref = PBLT_ref<uint32_t>;

void analyze_image(PNG_image& src, Succ_node& tree_root)
{
  auto height = src.get_height();
  auto width = src.get_width();

  auto shared_rows = src.get_data();
  auto raw_rows = shared_rows.get();

  deque<Succ_ref> insert_nodes;
  size_t const MAX_SIZE = 5;

  for(size_t y = 0; y < height; y++)
  {
    insert_nodes.clear();
    insert_nodes.push_back(Succ_ref(tree_root));

    png_byte* row = raw_rows[y];
    for(size_t x = 0; x < width; x++)
    {
      png_byte c_r = row[x * 4 + 0];
      png_byte c_g = row[x * 4 + 1];
      png_byte c_b = row[x * 4 + 2];
      long color = c_r << 24 | c_g << 16 | c_b << 8 | 255;

      size_t size = insert_nodes.size();
      for(size_t i = 0; i < size; i++)
      {
        Succ_ref& r = insert_nodes.front();
        insert_nodes.pop_front();
        Succ_node& nxt = r.visit(color & 0b11111000111110001111100011111000);
        insert_nodes.push_back(Succ_ref(nxt));
      }
      insert_nodes.push_front(Succ_ref(tree_root));
      if(insert_nodes.size() > MAX_SIZE)
        insert_nodes.pop_back();
    }
  }
}

void generate_image(Succ_node& tree, PNG_image& dst)
{
  auto height = dst.get_height();
  auto width = dst.get_width();

  auto shared_rows = dst.get_data();
  auto raw_rows = shared_rows.get();

  for(size_t y = 0; y < height; y++)
  {
    png_byte* row = raw_rows[y];
    for(size_t x = 0; x < width; x++)
    {
      Succ_node& r = tree.find_random((uint32_t)rand());
      uint32_t clr = r.get_key();

      row[x * 4 + 0] = clr >> 24 & 0xFF;
      row[x * 4 + 1] = clr >> 16 & 0xFF;
      row[x * 4 + 2] = clr >> 8 & 0xFF;
    }
  }
}

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

  Succ_node tree_root;
  analyze_image(source, tree_root);
  PNG_image destination(PNG_image::next_free_filename(filename), source);
  generate_image(tree_root, destination);
  destination.unload();

  log() << "Exit" << endl;
}
