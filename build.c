#define NOM_IMPLEMENTATION
#include "nom.h"

int main(int argc, char* argv[]) {
  // nom_logger.show_debug = OFF;
  rebuild(argc, argv, __FILE__, "gcc");
  IS_LIBRARY_MODIFIED("nom.h", "build.c", "gcc");
  nom_log(NOM_WARN, "hello");
  nom_log(NOM_DEBUG, "hello");
  nom_log(NOM_INFO, "hello");
  nom_log(NOM_PANIC, "hello");
  nom_log(NOM_NONE, "hello");
  return 0;
  int end = ends_substr("helloaskjldalksjfjasdfkljakjfdashjkfdsahk", "elloaskjldalksjfjasdfkljakjfdashjkfdsahk");
  if(end) {
    printf("has\n");
  } else
    printf("not\n");
}
