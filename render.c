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

void body_html(lua_State *L, Meta **, int);

void render_html(lua_State *L, Meta **array_meta, int array_size) {
    char *basedir = luaL_check_string(L, 1);
    printf("<html>");
    printf("<head>");

    char *style = read_resource(basedir, "style.css");
    printf( "<style>%s</style>", style); // css
    free(style);

    char *jquery = read_resource(basedir, "jquery-1.11.0.min.js");
    printf("<script>%s</script>", jquery); // jquery
    free(jquery);

    printf("</head>");
    printf("<body>");

    body_html(L, array_meta, array_size);

    char *profile = read_resource(basedir, "profile.js");
    printf("<script>%s</script>", profile); // js
    free(profile);

    printf("</html>");
}

void body_html(lua_State *L, Meta **array_meta, int array_size) {
    size_t buffsize = (strlen(HTML_FRAME) + strlen(HTML_FRAME_CLOSE) + 1024);
    char *out = (char *) malloc(buffsize * sizeof(char));
    if (!out) lua_error(L, "out of memory!");

    int index;
    Meta *meta;
    char *extra_class;

    for (index = 0; index < array_size; index++) {
        meta = array_meta[index];
        extra_class = !meta->children->list ? "no_children " : "";
        sprintf(out, HTML_FRAME,
                extra_class,
                meta->measure->time_spent,
                meta->fun_name,
                meta->fun_scope,
                meta->func_file,
                meta->line
        );
        printf(out);
        memset(out, 0, buffsize);
        if (meta->children->list) {
            body_html(L, meta->children->list, meta->children->index);
        }
        printf(HTML_FRAME_CLOSE);
    }
    free(out);
}