//
// Created by alex on 16/10/2015.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lauxlib.h>
#include "render.h"
#include "measure.h"

#ifdef _WIN32
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif

static char *PAGE =
"<html>"
    "<head>"
        "<style>%s</style>" // css
        "<script>%s</script>" // jquery
    "</head>"
    "<body>"
        "%s" // body
        "<script>%s</script>" // js
    "</body>"
"</html>";


static char *HTML_FRAME =
"<div class=\"frame %s\" data-time=\"\" date-parent-time=\"\">" // extra-class
    "<div class=\"frame-info\">"

        "<span class=\"lineno\">%i| </span>"
        "<span class=\"func_name\">%s </span>"
        "<span class=\"func_scope\">(%s)</span>"
        "<span class=\"func_file\">(%s)</span>"
        "<span class=\"time\"> %.3fs</span>" // time spent ?

        "<span class=\"total-percent\"></span>"
        "<!--<span class=\"parent-percent\"></span>-->"
        "<span class=\"function\"></span>"
        "<span class=\"code-position\"></span>" // position
    "</div>"
"<div class=\"frame-children\">";

static char *HTML_FRAME_CLOSE = "</div></div>";

static char *read_resource(char *basedir, char *filename) {
    char *format = "%sresources%s%s";
    char *fullpath = (char *) malloc(strlen(filename) + strlen(format) + 2);
    sprintf(fullpath, format, basedir, SEPARATOR, filename);

    FILE *fp;

    fp = fopen(fullpath, "rb");
    char *data = NULL;
    if (fp) {
        fseek(fp, 0, SEEK_END);  // get file size (cross way)
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        data = (char *) malloc((size_t) file_size + 1);
        fread(data, 1, (size_t) file_size, fp);
        data[file_size] = '\0';
        fclose(fp);
    }
    return data;
}

char *as_html(lua_State *L, Meta *meta, bool is_child);

char *render_html(lua_State *L, Meta *array_metadata, int array_size) {
    char *basedir = luaL_check_string(L, 1);

    char *style = read_resource(basedir, "style.css");
    char *profile = read_resource(basedir, "profile.js");
    char *jquery = read_resource(basedir, "jquery-1.11.0.min.js");

    int buffsize = 2048;
    char *body = (char *) malloc(buffsize * sizeof(char));
    if (!body) lua_error(L, "body: out of memory!");
    body[0] = '\0'; // empty string

    char *output;
    Meta meta;
    Meta *mnext;
    int nextindex;
    int index;

    for (index = 0; index < array_size; index++) {
        meta = array_metadata[index];

        output = as_html(L, &meta, meta.stack_level > 0);
        nextindex = index + 1;

        mnext = nextindex < array_size ? &array_metadata[nextindex] : NULL;

        if (!mnext || mnext->stack_level == 0) {
            strcat(output, HTML_FRAME_CLOSE);
        }
        if ((strlen(body) + strlen(output)) > buffsize - 1) {
            buffsize = buffsize + strlen(output) + 5120;
            body = (char *) realloc(body, buffsize * sizeof(char));
            if (!body) lua_error(L, "body: out of memory!");
        }

        strncat(body, output, strlen(output));
        free(output);
    }
    int page_size = strlen(PAGE)   + strlen(style) + strlen(profile) +
                    strlen(jquery) + strlen(body);
    char *page = (char *) malloc((page_size + 1) * sizeof(char));
    if (!page) lua_error(L, "page: out of memory!");

    sprintf(page, PAGE, style, jquery, body, profile);

    free(style);
    free(jquery);
    free(profile);
    free(body);

    return page;
}


char *as_html(lua_State *L, Meta *meta, bool is_child) {
    char *extra_class = is_child ? "no_children " : "";

    size_t buffsize = strlen(extra_class) + strlen(HTML_FRAME) +
                      strlen(HTML_FRAME_CLOSE);
    char *output = (char *) malloc((buffsize + 5120) * sizeof(char));
    if (!output) lua_error(L, "output: out of memory!");

    sprintf(output, HTML_FRAME,
            extra_class,
            meta->line,
            meta->fun_name,
            meta->fun_scope,
            meta->func_file,
            meta->measure->time_spent
    );
    return output;
}