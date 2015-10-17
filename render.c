//
// Created by alex on 16/10/2015.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lauxlib.h>
#include <assert.h>
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
"<div class=\"frame application %s\" data-time=\"\" date-parent-time=\"\">" // extra-class
    "<div class=\"frame-info\">"
        "<span class=\"time\">%.3fs</span>" // time spent ?
        "<!--<span class=\"total-percent\"></span>-->"
        "<!--<span class=\"parent-percent\"></span>-->"
        "<span class=\"function\">%s (%s)</span>"
        "<span class=\"code-position\">%s:%i</span>" // position
    "</div>"
"<div class=\"frame-children\">";

static char *HTML_FRAME_CLOSE = "</div></div>";

static char *read_resource(char *basedir, char *filename) {
    char *format = "%sresources%s%s";
    int buffsize = strlen(format) + strlen(basedir) + strlen(filename) + 1;
    char *fullpath = (char *) malloc(buffsize *sizeof(char));
    sprintf(fullpath, format, basedir, SEPARATOR, filename);

    FILE *fp = fopen(fullpath, "rb");
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
    assert(data != NULL);
    return data;
}

char *as_html(lua_State *L, Meta **array_meta, int array_size, char *output, int outsize);

char *render_html(lua_State *L, Meta **array_meta, int array_size) {
    char *basedir = luaL_check_string(L, 1);

    char *style = read_resource(basedir, "style.css");
    char *profile = read_resource(basedir, "profile.js");
    char *jquery = read_resource(basedir, "jquery-1.11.0.min.js");

    size_t buffsize = (strlen(HTML_FRAME) + strlen(HTML_FRAME_CLOSE) + 1024) * 25;
    char *body = (char *) malloc(buffsize * sizeof(char));
    if (!body) lua_error(L, "output: out of memory!");
    body[0] = '\0'; // empty string

    as_html(L, array_meta, array_size, body, buffsize);

    //printf("%s\n\n\n", body);

    int page_size = strlen(PAGE) + strlen(style) + strlen(profile) +
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


char *as_html(lua_State *L, Meta **array_meta, int array_size, char *output, int outsize) {
    int index;
    Meta *meta;
    char *extra_class;
    char *out;

    size_t buffsize = (strlen(HTML_FRAME) + strlen(HTML_FRAME_CLOSE) + 1024);
    int buff;

    for (index = 0; index < array_size; index++) {
        meta = array_meta[index];

        extra_class = !meta->children->list ? "no_children " : "";
        out = (char *) malloc((buffsize + strlen(extra_class)) * sizeof(char));
        if (!out) lua_error(L, "out of memory!");

        sprintf(out, HTML_FRAME,
                extra_class,
                meta->measure->time_spent,
                meta->fun_name,
                meta->fun_scope,
                meta->func_file,
                meta->line
        );
        buff = strlen(output) + strlen(out);
        if (buff > outsize) {
            outsize = outsize + buff;
            output = realloc(output, outsize * sizeof(char));
            if (!output) lua_error(L, "out of memory!");
        }
        strcat(output, out);
        if (meta->children->list) {
            as_html(L, meta->children->list, meta->children->index, output, outsize);
        }
        buff = strlen(output) + strlen(HTML_FRAME_CLOSE);

        if (buff > outsize) {
            outsize = outsize + buff;
            output = realloc(output, outsize * sizeof(char));
            if (!output) lua_error(L, "out of memory!");
        }
        strcat(output, HTML_FRAME_CLOSE);
    }
    return output;
}