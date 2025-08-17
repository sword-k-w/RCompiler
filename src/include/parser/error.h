#ifndef ERROR_H
#define ERROR_H

#include <string>

class Error : public std::exception {
public:
  Error() = delete;
  Error(const std::string &);
  std::string_view Info() const;
private:
  std::string_view info_;
};

#endif //ERROR_H
