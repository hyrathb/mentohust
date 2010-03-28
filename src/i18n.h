/*
 * i18n.h
 *
 *  Created on: 2010-3-29
 *      Author: cai
 */

#ifndef I18N_H_
#define I18N_H_


#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#else
#define _(String) (String)
#define N_(String) (String)
#define setlocale(x,y)
#define textdomain(x)
#define bindtextdomain(x,y)
#endif


#endif /* I18N_H_ */
