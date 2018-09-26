#include "http_msg_creator.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

namespace beam {

namespace {

struct CurrentOutput {
    CurrentOutput(io::SerializedMsg& out, io::SerializedMsg** currentMsg)
        : _currentMsg(currentMsg)
    {
        out.clear();
        *_currentMsg = &out;
    }
    ~CurrentOutput() { *_currentMsg = 0; }

    io::SerializedMsg** _currentMsg;
};

bool write_fmt(FragmentWriter& fw, const char* fmt, ...) {
    static const int MAX_BUFSIZE = 4096;
    char buf[MAX_BUFSIZE];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, MAX_BUFSIZE, fmt, ap);
    va_end(ap);
    if (n < 0) {
        return false;
    }
    fw.write(buf, n);
    return true;
}

bool create_message(
    io::SerializedMsg& out,
    FragmentWriter& fw,
    const HeaderPair* headers, size_t num_headers,
    const char* content_type,
    const io::SharedBuffer& body
) {
    for (size_t i=0; i<num_headers; ++i) {
        const HeaderPair& p = headers[i];
        assert(p.head);
        if (!p.is_number) {
            assert(p.content_str);
            if (!write_fmt(fw, "%s: %s\r\n", p.head, p.content_str)) return false;
        } else {
            if (!write_fmt(fw, "%s: %lu\r\n", p.head, p.content_num)) return false;
        }
    }

    if (body.size > 0) {
        assert(content_type != nullptr);
        if (!write_fmt(fw, "%s: %s\r\n", "Content-Type", content_type)) return false;
        if (!write_fmt(fw, "%s: %lu\r\n", "Content-Length", (unsigned long)body.size)) return false;
        fw.write("\r\n", 2);
        fw.finalize();
        out.push_back(body);
    } else {
        fw.write("\r\n", 2);
        fw.finalize();
    }

    return true;
}

} //namespace

bool HttpMsgCreator::create_request(
    io::SerializedMsg& out,
    const char* method,
    const char* path,
    const HeaderPair* headers, size_t num_headers,
    int http_minor_version,
    const char* content_type,
    const io::SharedBuffer& body
) {
    CurrentOutput co(out, &_currentMsg);

    assert(method != nullptr);
    assert(path != nullptr);
    if (!write_fmt(_fragmentWriter, "%s %s HTTP/1.%d\r\n", method, path, http_minor_version)) return false;

    return create_message(out, _fragmentWriter, headers, num_headers, content_type, body);
}

bool HttpMsgCreator::create_response(
    io::SerializedMsg& out,
    int code,
    const char* message,
    const HeaderPair* headers, size_t num_headers,
    int http_minor_version,
    const char* content_type,
    const io::SharedBuffer& body
) {
    CurrentOutput co(out, &_currentMsg);

    assert(message != nullptr);
    if (!write_fmt(_fragmentWriter, "HTTP/1.%d %d %s\r\n", http_minor_version, code, message)) return false;

    return create_message(out, _fragmentWriter, headers, num_headers, content_type, body);
}

} //namepsace