/* This is a managed file. Do not delete this comment. */

#include <corto/httprouter/httprouter.h>

int16_t httprouter_service_onRequest(
    httprouter_service this,
    httpserver_HTTP_Connection c,
    httpserver_HTTP_Request *r,
    const char *uri)
{
    corto_log_push("httprouter");
    int16_t result = httprouter_service_forward(this, r, uri);
    corto_log_pop();
    return result;
}

int16_t httprouter_service_construct(
    httprouter_service this)
{
    httpserver_Service(this)->redirectEndpointToPath = true;
    return corto_super_construct(this);
}

int16_t httprouter_service_forward(
    httprouter_service this,
    httpserver_HTTP_Request *r,
    const char *uri)
{
    corto_string resultStr = NULL;
    corto_any result = {corto_type(corto_string_o), &resultStr, FALSE};
    corto_any param = {corto_type(httpserver_HTTP_Request_o), r, FALSE};
    corto_route route = NULL;

    char *uri_last_elem = strrchr(uri, '/');
    char *real_uri = (char*)uri;

    /* If slash is last character, strip it from string */
    if (uri_last_elem && !uri_last_elem[1] && uri_last_elem != uri) {
        real_uri = corto_strdup(uri);
        real_uri[uri_last_elem - uri] = '\0';
    }

    corto_debug("match uri '%s' to routes of '%s'",
        uri,
        corto_fullpath(NULL, this));

    if (corto_router_match(this, real_uri, param, result, &route)) {
        corto_catch(); /* Don't report error */
        corto_debug("request '%s' not matched to routes in '%s' of type '%s'",
            r->uri,
            corto_fullpath(NULL, this),
            corto_fullpath(NULL, corto_typeof(this)));
        goto nomatch;
    } else {
        if (httpserver_HTTP_Request_getStatus(r) < 400) {
            corto_trace("matched '%s' with '%s'", uri, corto_fullpath(NULL, route));
            if (resultStr) {
                httpserver_HTTP_Request_reply(r, resultStr);
                corto_debug("result: '%s'", resultStr);
                corto_dealloc(resultStr);
            }
        } else {
            /* If route returned a 404, router did not match */
            goto nomatch;
        }
    }

    if (real_uri != uri) free(real_uri);
    return 1;
nomatch:
    if (real_uri != uri) free(real_uri);
    return 0;
}
