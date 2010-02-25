
#ifndef _LIBGETTEXT_H
#define _LIBGETTEXT_H 1

/* NLS can be disabled through the configure --disable-nls option.  */
#if ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String)  gettext(String)
#define gettext_noop(String) (String)
#define N_(String) gettext_noop (String)
#else
#define _(String) (String)
#define N_(String) (String)
#define setlocale(x,y)
#define textdomain(x)
#define bindtextdomain(x,y)
#endif

#ifdef DEBUG
#define textdomain(domian) \
	do { textdomain(domian) ; \
		bindtextdomain(GETTEXT_PACKAGE, "./usr/share/locale"); \
	}while(0)

#endif

#endif /* _LIBGETTEXT_H */
