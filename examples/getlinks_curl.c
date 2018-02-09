/*
 * getlinks_curl - Example program using Haut
 *
 * https://github.com/mickymuis/haut-html
 *
 * Micky Faas <micky@edukitty.org>
 * Copyright 2017-2018
 * Leiden Institute of Advanced Computer Science, The Netherlands
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <haut/haut.h>
#include <haut/tag.h>

/* Usage:
 * getlinks_curl [url]
 *
 * Obtains all links of type <a href=% from an HTML-page obtained using libCURL.
 */

