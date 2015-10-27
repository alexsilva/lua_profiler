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

static char *HTML_FRAME_CLOSE = "</div></div>";

static char *read_filecontent(char *filename) {
    FILE *fp = fopen(filename, "rb");
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

static char *formatpath(char *format, char *basedir, char *filename) {
    char *fullpath = malloc(strlen(format) + strlen(basedir) + strlen(filename) + 1);
    sprintf(fullpath, format, basedir, SEPARATOR, SEPARATOR, filename);
    return fullpath;
}

static char *read_resource(char *basedir, char *filename) {
    char *fullpath = formatpath("%sprofiler%sresources%s%s", basedir, filename);
    char *data = read_filecontent(fullpath);
    free(fullpath);
    return data;
}

static char *read_template(char *basedir, char *filename) {
    char *fullpath = formatpath("%sprofiler%stemplates%s%s", basedir, filename);
    char *data = read_filecontent(fullpath);
    free(fullpath);
    return data;
}

void body_html(lua_State *L, Meta **, int, char *);

void render_html(lua_State *L, Meta **array, int size) {
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
    char *html_frame = read_template(basedir, "render.html");
    body_html(L, array, size, html_frame);
    free(html_frame);
    printf("</body>");

    char *profile = read_resource(basedir, "profile.js");
    printf("<script>%s</script>", profile); // js
    free(profile);

    printf("</html>");
}

void body_html(lua_State *L, Meta **array_meta, int array_size, char *html_frame) {
    size_t buffsize = (strlen(html_frame) + strlen(HTML_FRAME_CLOSE) + 1024);
    char *out = (char *) malloc(buffsize * sizeof(char));
    if (!out) lua_error(L, "out of memory!");

    int index;
    Meta *meta;
    char *extra_class;

    for (index = 0; index < array_size; index++) {
        meta = array_meta[index];
        extra_class = !meta->children->list ? "no_children " : "";
        sprintf(out, html_frame,
                extra_class,
                meta->measure->time_spent,
                meta->fun_name,
                meta->fun_scope,
                meta->func_file,
                meta->line
        );
        printf("%s", out);
        memset(out, 0, buffsize);
        if (meta->children->list) {
            body_html(L, meta->children->list, meta->children->index, html_frame);
        }
        printf("%s", HTML_FRAME_CLOSE);
    }
    free(out);
}

static char *repeat_str(char *str, size_t count) {
    if (count == 0) return NULL;
    char *ret = malloc((strlen(str) * count) + 1);
    if (ret == NULL) return NULL;
    strcpy(ret, str);
    while (--count > 0) {
        strcat(ret, str);
    }
    return ret;
}

static void _render_text(lua_State *L, Meta **array, int size,
                         char *texttpl, char *offsetc, char *breakln) {
    Meta *meta;
    int index;
    char *offsettext;
    for (index = 0; index < size; index++) {
        meta = array[index];

        offsettext = repeat_str(offsetc, (size_t) meta->stack_level);

        printf(texttpl,
               !offsettext ? "" : offsettext,
               meta->measure->time_spent,
               meta->fun_name,
               meta->fun_scope,
               meta->func_file,
               meta->line,
               breakln
        );
        if (offsettext) free(offsettext);
        if (meta->children->list) {
            _render_text(L, meta->children->list, meta->children->index,
                         texttpl, offsetc, breakln);
        }
    }
}

void render_text(lua_State *L, Meta **array, int size) {
    char *basedir = luaL_check_string(L, 1);
    // Lua args
    lua_Object lobj = lua_getparam(L, 2);
    char *breakln = lua_isstring(L, lobj) ? lua_getstring(L, lobj) : "\n";

    lobj = lua_getparam(L, 3);
    char *offsetc = lua_isstring(L, lobj) ? lua_getstring(L, lobj) : "\t";

    char *texttpl = read_template(basedir, "render.txt");
    _render_text(L, array, size, texttpl, offsetc, breakln);
    free(texttpl);
}


void _render_json(lua_State *L, Meta **array, int size, char *jsontmpl) {
    Meta *meta;
    int index;
    for (index = 0; index < size; index++) {
        meta = array[index];
        printf(jsontmpl,
               !meta->children->list ? "no_children" : "",
               meta->measure->time_spent,
               meta->fun_name,
               meta->fun_scope,
               meta->func_file,
               meta->line
        );
        if (meta->children->list) {
            _render_json(L, meta->children->list, meta->children->index, jsontmpl);
        }
        printf("]}");
    }
}

void render_json(lua_State *L, Meta **array, int size) {
    char *basedir = luaL_check_string(L, 1);
    char *jsontmpl = read_template(basedir, "render.json");
    _render_json(L, array, size, jsontmpl);
}