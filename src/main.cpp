

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "pblt.hpp"
#include "png_image.hpp"

using namespace std;

ostream& log(void) { return cout << "[MAIN] "; }

ostream& err(void) { return cerr << "[MAIN] "; }

int main(int argc, char* argv[])
{
  std::unique_ptr<PBLT<uint32_t, uint32_t>> tree;
  for(int i = 0; i < 100; i++)
  {
    auto& res = PBLT<uint32_t, uint32_t>::visit(tree, 42);
    res += 10;
  }

  for(int i = 0; i < 10; i++)
  {
    auto& res = PBLT<uint32_t, uint32_t>::visit(tree, 43);
    res += 5;
  }

  for(int i = 0; i < 1000; i++)
  {
    cout << tree->find_random((uint32_t)rand()) << endl;
  }

  return 0;

  if(argc < 2)
  {
    log() << "Please supply image filename ____.png" << endl;
    return 1;
  }

  string filename(argv[1]);

  PNG_image source(filename + ".png");
  source.load();

  auto height = source.get_height();
  auto width = source.get_width();

  PNG_image destination(PNG_image::next_free_filename(filename), source);

  auto shared_rows = destination.get_data();
  auto raw_rows = shared_rows.get();
  for(size_t y = 0; y < height; y++)
  {
    png_byte* row = raw_rows[y];
    for(size_t x = 0; x < width; x++)
    {
      png_byte* ptr = &(row[x * 4]);
      ptr[0] &= 0b11111000;
      ptr[1] &= 0b11111000;
      ptr[2] &= 0b11111000;
    }
  }

  destination.unload();

  log() << "Exit" << endl;
}
