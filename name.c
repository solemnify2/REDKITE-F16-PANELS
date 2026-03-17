#include "usb_names.h"

struct usb_string_descriptor_struct usb_string_product_name = {
  2 + 18 * 2,  // 2 + (글자수 * 2)
  3,
  {'R','e','d','k','i','t','e',' ','F','1','6',' ','P','a','n','e','l','s'}
};
