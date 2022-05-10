#include <log_utils.hpp>

// compile using
// g++ -g -O0 -std=c++11  -I. -o lu-test log_utils_test.cpp
int main(int argc, char **argv)
{
    logu::logger foo;

    // add an message log (reference useing key "foo")
    foo.add_file("mlog","foo.mlog","a+",true);
    // add a binary log
    foo.add_file("blog","foo.blog","a+",true);

    // set destinations and formats
    // optionally, set according to application options

    // profile destination keys
    std::vector<std::string> debug_keys = {"stderr"};
    // intentionally omit verbose (should use default profile)
    // std::vector<std::string> verbose_keys = {"stderr"};
    std::vector<std::string> info_keys = {"stderr"};
    std::vector<std::string> event_keys = {"mlog"};
    std::vector<std::string> warn_keys = {"stderr","mlog"};
    std::vector<std::string> error_keys = {"stderr","mlog"};
    std::vector<std::string> dfl_keys = {"stderr","mlog"};

    // profile formats
    flag_var<uint32_t> rec_fmt = (logu::LF_TIME_ISO8601 | logu::LF_LVL_SHORT | logu::LF_SEP_COMMA | logu::LF_DEL_UNIX);
    flag_var<uint32_t> dfl_fmt = (logu::LF_TIME_POSIX_MS | logu::LF_SEP_COMMA | logu::LF_DEL_UNIX);

    // define profiles per level
    foo.set_profile(logu::LL_DEBUG, debug_keys, rec_fmt);
    // intentionally omit verbose (should use default profile)
    // foo.set_profile(logu::LL_VERBOSE, verbose_keys, rec_fmt);
    foo.set_profile(logu::LL_INFO, info_keys, rec_fmt);
    foo.set_profile(logu::LL_EVENT, event_keys, rec_fmt);
    foo.set_profile(logu::LL_WARN, warn_keys, rec_fmt);
    foo.set_profile(logu::LL_ERR, error_keys, rec_fmt);
    // if default profile is unset, just goes to stderr as-is
    foo.set_profile(logu::LL_DFL, dfl_keys, dfl_fmt);

    // profile outputs
    foo.pdebug("%s:%d debug msg", __func__, __LINE__);
    foo.pwarn("%s:%d warn msg", __func__, __LINE__);
    foo.pverbose("%s:%d verbose msg", __func__, __LINE__);
    foo.pevent("%s:%d event msg", __func__, __LINE__);
    foo.pinfo("%s:%d info msg", __func__, __LINE__);
    foo.perror("%s:%d error msg", __func__, __LINE__);

    // user defined output
    foo.ulog("mlog","freestyling!!\n");

    // log binary
    std::string x("domo arigatoo\n");
    std::string y("mr roboto\t");
    std::string z("domo\n");
    foo.blog("blog",(uint8_t *)x.c_str(),x.length());
    foo.blog("blog",(uint8_t *)y.c_str(),y.length());
    foo.blog("blog",(uint8_t *)z.c_str(),z.length());

    // apply profile on the fly (without changing existing profile defs)
    std::vector<std::string> alt_keys = {"stderr","mlog"};
    flag_var<uint32_t> alt_fmt = (logu::LF_TIME_POSIX_S | logu::LF_LVL_LONG | logu::LF_SEP_DASH | logu::LF_DEL_UNIX);

    logu::log_profile aprof(logu::LL_INFO, alt_keys, alt_fmt);

    foo.plog(aprof, "%s:%d alt msg", __func__, __LINE__);

    return 0;
}
