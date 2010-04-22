/*
 * i18n.h
 *
 *  Created on: 2010-3-29
 *      Author: cai
 */

#ifndef I18N_H_
#define I18N_H_
#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define GETTEXT_PACKAGE "mentohust"
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else
#define _(String) (String)
#define N_(String) (String)
#endif

#endif /* I18N_H_ */
