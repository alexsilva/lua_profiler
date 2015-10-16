//
// Created by alex on 16/10/2015.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "measure.h"


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


static char *ITEM_HTML_START =
"<div class=\"frame {%s}\" data-time=\"{time}\" date-parent-time=\"{parent_proportion}\">" // extra-class
    "<div class=\"frame-info\">"
        "<span class=\"time\">{%.3f}s</span>" // time spent ?
            "<span class=\"total-percent\">{total_proportion}</span>"
            "<!--<span class=\"parent-percent\">{parent_proportion}</span>-->"
        "<span class=\"function\">{function}</span>"
    "<span class=\"code-position\">{code_position}</span>"
"</div>"
"<div class=\"frame-children\">";

static char *ITEM_HTML_END = "</div></div>";


static char *read_resource(char *filename)
{
    char *resource_dir = "..\\resources";
    char *fullpath = (char *) malloc(strlen(resource_dir) + strlen(filename) + 2);

    sprintf(fullpath, "%s/%s", resource_dir, filename);

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

char *render_html(Meta *array_metadata, int array_size) {
    int index;
    Meta meta;

    char *style = read_resource("style.css");
    char *profile = read_resource("profile.js");
    char *jquery = read_resource("jquery-1.11.0.min.js");

    char *body = "<p>BODY CONTENT</p>";

    int page_size = strlen(body) + strlen(PAGE) + strlen(style) + strlen(profile) + strlen(jquery) + 1;

    char *page = (char *) malloc(page_size * sizeof(char));

    //for (index= 0; index < array_size; index++) {
    //    meta = array_metadata[index];
    //    as_html(meta, meta.stack_level > 0);
    //}

    sprintf(page, PAGE, style, jquery, body, profile);

    free(style);
    free(jquery);
    free(profile);

    return page;
}


char *as_html(Meta meta, bool is_child) {
    int strsize = strlen(ITEM_HTML_START) + strlen(ITEM_HTML_END) + 1;

    char *output = (char *) malloc(strsize * sizeof(char));
    char *extra_class = is_child ? "no_children " : "";

    sprintf(output, "");

    return output;
}