#include <FreeRTOS.h>
#include <task.h>
#include <stream_buffer.h>
#include <string.h>
#include <stdio.h>
#include <io.h>

static void strl(char const* c, size_t l) {
	size_t sent = 0;
	while(sent < l) {
		sent += cdcacm_write(c + sent, l - sent);
	}
}

static void str(char const* c) {
	strl(c, strlen(c));
}

static void esc() {
	str("\033[");
}

static void num(int i) {
	char num[11];
	int len = snprintf(num, sizeof(num), "%d", i);
	strl(num, len);
}

static void chr(char c) {
	cdcacm_write(&c, 1);
}

static char get() {
	char buf;
	cdcacm_read(&buf, 1);
	return buf;
}

static void insert(char* buf, size_t len, int pos, char c) {
	char tmp[len - pos - 1];
	memcpy(tmp, buf + pos, len - pos - 1);
	memcpy(buf + pos + 1, tmp, len - pos - 1);
	buf[pos] = c;
}

static void delete(char* buf, size_t len, int pos) {
	char tmp[len - pos - 1];
	memcpy(tmp, buf + pos + 1, len - pos - 1);
	memcpy(buf + pos, tmp, len - pos - 1);
}

size_t term_getline(char* buf, size_t maxlen) {
	int pos = 0, right = 0;

	memset(buf, 0, maxlen);

	char c;

	str("\033[2K");
	str("\033[1G");

	while(1) {
		c = get();
		if (c == 27) /* ESC */ {
			c = get();
			if (c == 91) {
				c = get();
				if (c == 68) /* LEFT */ {
					if (pos > 0) {
						pos--;
					}
				} else if(c == 67) /* RIGHT */ {
					if (pos < right && right < maxlen - 1) {
						pos++;
					}
				}
			}
		} else if(c == 127) {
			if (pos > 0) {
				delete(buf, maxlen, pos - 1);
				--right;
				--pos;
			}
		} else if(c == '\r' || c == '\n') {
			chr(c);
			return right;
		} else {
			if (right < maxlen - 1) {
				insert(buf, maxlen, pos, c);
				++right;
				++pos;
			}
		}

		str("\033[2K");
		str("\033[1G");
		strl(buf, right);
		esc();num(pos + 1);str("G");
	}

}

int __io_putchar(int ch) {
	chr(ch);
	return 1;
}

int __io_getchar(void) {
	return get();
}
