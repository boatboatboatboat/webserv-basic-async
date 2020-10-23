//
// Created by boat on 8/27/20.
//

#ifndef WEBSERV_HTTPSTATUS_HPP
#define WEBSERV_HTTPSTATUS_HPP

#include <iostream>
#include <string>

namespace http {

struct Status {
    unsigned short code;
    std::string_view message;
};

namespace status {

    /*
    The 100 (Continue) status code indicates that the initial part of a
        request has been received and has not yet been rejected by the
        server.  The server intends to send a final response after the
    request has been fully received and acted upon.

    When the request contains an Expect header field that includes a
    100-continue expectation, the 100 response indicates that the server
        wishes to receive the request payload body, as described in
    Section 5.1.1.  The client ought to continue sending the request and
        discard the 100 response.

    If the request did not contain an Expect header field containing the
    100-continue expectation, the client can simply discard this interim
        response.
*/
    inline const Status CONTINUE { 100, "Continue" };

    /*
 The 101 (Switching Protocols) status code indicates that the server
   understands and is willing to comply with the client's request, via
   the Upgrade header field (Section 6.7 of [RFC7230]), for a change in
   the application protocol being used on this connection.  The server
   MUST generate an Upgrade header field in the response that indicates
   which protocol(s) will be switched to immediately after the empty
   line that terminates the 101 response.

   It is assumed that the server will only agree to switch protocols
   when it is advantageous to do so.  For example, switching to a newer
   version of HTTP might be advantageous over older versions, and
   switching to a real-time, synchronous protocol might be advantageous
   when delivering resources that use such features.
*/
    inline const Status SWITCHING_PROTOCOLS { 101, "Switching Protocols" };

    /*
The 200 (OK) status code indicates that the request has succeeded.
   The payload sent in a 200 response depends on the request method.
   For the methods defined by this specification, the intended meaning
   of the payload can be summarized as:

   GET  a representation of the target resource;

   HEAD  the same representation as GET, but without the representation
      data;

   POST  a representation of the status of, or results obtained from,
      the action;

   PUT, DELETE  a representation of the status of the action;

   OPTIONS  a representation of the communications options;

   TRACE  a representation of the request message as received by the end
      server.

   Aside from responses to CONNECT, a 200 response always has a payload,
   though an origin server MAY generate a payload body of zero length.
   If no payload is desired, an origin server ought to send 204 (No
   Content) instead.  For CONNECT, no payload is allowed because the
   successful result is a tunnel, which begins immediately after the 200
   response header section.

   A 200 response is cacheable by default; i.e., unless otherwise
   indicated by the method definition or explicit cache controls (see
   Section 4.2.2 of [RFC7234]).
*/
    inline const Status OK { 200, "OK" };

    /*
   The 201 (Created) status code indicates that the request has been
   fulfilled and has resulted in one or more new resources being
   created.  The primary resource created by the request is identified
   by either a Location header field in the response or, if no Location
   field is received, by the effective request URI.

   The 201 response payload typically describes and links to the
   resource(s) created.  See Section 7.2 for a discussion of the meaning
   and purpose of validator header fields, such as ETag and
   Last-Modified, in a 201 response.
*/
    inline const Status CREATED { 201, "Created" };

    /*
   The 202 (Accepted) status code indicates that the request has been
   accepted for processing, but the processing has not been completed.
   The request might or might not eventually be acted upon, as it might
   be disallowed when processing actually takes place.  There is no
   facility in HTTP for re-sending a status code from an asynchronous
   operation.

   The 202 response is intentionally noncommittal.  Its purpose is to
   allow a server to accept a request for some other process (perhaps a
   batch-oriented process that is only run once per day) without
   requiring that the user agent's connection to the server persist
   until the process is completed.  The representation sent with this
   response ought to describe the request's current status and point to
   (or embed) a status monitor that can provide the user with an
   estimate of when the request will be fulfilled.
*/
    inline const Status ACCEPTED { 202, "Accepted" };

    /*
   The 203 (Non-Authoritative Information) status code indicates that
   the request was successful but the enclosed payload has been modified
   from that of the origin server's 200 (OK) response by a transforming
   proxy (Section 5.7.2 of [RFC7230]).  This status code allows the
   proxy to notify recipients when a transformation has been applied,
   since that knowledge might impact later decisions regarding the
   content.  For example, future cache validation requests for the
   content might only be applicable along the same request path (through
   the same proxies).

   The 203 response is similar to the Warning code of 214 Transformation
   Applied (Section 5.5 of [RFC7234]), which has the advantage of being
   applicable to responses with any status code.

   A 203 response is cacheable by default; i.e., unless otherwise
   indicated by the method definition or explicit cache controls (see
   Section 4.2.2 of [RFC7234]).
*/
    inline const Status NON_AUTHORITATIVE_INFORMATION { 203, "Non-Authoritative Information" };

    /*
   The 204 (No Content) status code indicates that the server has
   successfully fulfilled the request and that there is no additional
   content to send in the response payload body.  Metadata in the
   response header fields refer to the target resource and its selected
   representation after the requested action was applied.

   For example, if a 204 status code is received in response to a PUT
   request and the response contains an ETag header field, then the PUT
   was successful and the ETag field-value contains the entity-_tag for
   the new representation of that target resource.

   The 204 response allows a server to indicate that the action has been
   successfully applied to the target resource, while implying that the
   user agent does not need to traverse away from its current "document
   view" (if any).  The server assumes that the user agent will provide
   some indication of the success to its user, in accord with its own
   interface, and apply any new or updated metadata in the response to
   its active representation.

   For example, a 204 status code is commonly used with document editing
   interfaces corresponding to a "save" action, such that the document
   being saved remains available to the user for editing.  It is also
   frequently used with interfaces that expect automated data transfers
   to be prevalent, such as within distributed version control systems.

   A 204 response is terminated by the first empty line after the header
   fields because it cannot contain a message body.

   A 204 response is cacheable by default; i.e., unless otherwise
   indicated by the method definition or explicit cache controls (see
   Section 4.2.2 of [RFC7234]).
*/
    inline const Status NO_CONTENT { 204, "No Content" };

    /*
   The 205 (Reset Content) status code indicates that the server has
   fulfilled the request and desires that the user agent reset the
   "document view", which caused the request to be sent, to its original
   state as received from the origin server.

   This response is intended to support a common data entry use case
   where the user receives content that supports data entry (a form,
   notepad, canvas, etc.), enters or manipulates data in that space,
   causes the entered data to be submitted in a request, and then the
   data entry mechanism is reset for the next entry so that the user can
   easily initiate another input action.

   Since the 205 status code implies that no additional content will be
   provided, a server MUST NOT generate a payload in a 205 response.  In
   other words, a server MUST do one of the following for a 205
   response: a) indicate a zero-length body for the response by
   including a Content-Length header field with a value of 0; b)
   indicate a zero-length payload for the response by including a
   Transfer-Encoding header field with a value of chunked and a message
   body consisting of a single chunk of zero-length; or, c) close the
   connection immediately after sending the blank line terminating the
   header section.
*/
    inline const Status RESET_CONTENT { 205, "Reset Content" };

    /*
   The 300 (Multiple Choices) status code indicates that the target
   resource has more than one representation, each with its own more
   specific identifier, and information about the alternatives is being
   provided so that the user (or user agent) can select a preferred
   representation by redirecting its request to one or more of those
   identifiers.  In other words, the server desires that the user agent
   engage in reactive negotiation to select the most appropriate
   representation(s) for its needs (Section 3.4).

   If the server has a preferred choice, the server SHOULD generate a
   Location header field containing a preferred choice's URI reference.
   The user agent MAY use the Location field value for automatic
   redirection.

   For request methods other than HEAD, the server SHOULD generate a
   payload in the 300 response containing a list of representation
   metadata and URI reference(s) from which the user or user agent can
   choose the one most preferred.  The user agent MAY make a selection
   from that list automatically if it understands the provided media
   type.  A specific format for automatic selection is not defined by
   this specification because HTTP tries to remain orthogonal to the
   definition of its payloads.  In practice, the representation is
   provided in some easily parsed format believed to be acceptable to
   the user agent, as determined by shared design or content
   negotiation, or in some commonly accepted hypertext format.

   A 300 response is cacheable by default; i.e., unless otherwise
   indicated by the method definition or explicit cache controls (see
   Section 4.2.2 of [RFC7234]).

   Note: The original proposal for the 300 status code defined the
   URI header field as providing a list of alternative
   representations, such that it would be usable for 200, 300, and
   406 responses and be transferred in responses to the HEAD method.
   However, lack of deployment and disagreement over syntax led to
   both URI and Alternates (a subsequent proposal) being dropped from
   this specification.  It is possible to communicate the list using
   a set of Link header fields [RFC5988], each with a relationship of
   "alternate", though deployment is a chicken-and-egg problem.
*/
    inline const Status MULTIPLE_CHOICES { 300, "Multiple Choices" };

    /*
   The 301 (Moved Permanently) status code indicates that the target
   resource has been assigned a new permanent URI and any future
   references to this resource ought to use one of the enclosed URIs.
   Clients with link-editing capabilities ought to automatically re-link
   references to the effective request URI to one or more of the new
   references sent by the server, where possible.

   The server SHOULD generate a Location header field in the response
   containing a preferred URI reference for the new permanent URI.  The
   user agent MAY use the Location field value for automatic
   redirection.  The server's response payload usually contains a short
   hypertext note with a hyperlink to the new URI(s).

      Note: For historical reasons, a user agent MAY change the request
      method from POST to GET for the subsequent request.  If this
      behavior is undesired, the 307 (Temporary Redirect) status code
      can be used instead.

   A 301 response is cacheable by default; i.e., unless otherwise
   indicated by the method definition or explicit cache controls (see
   Section 4.2.2 of [RFC7234]).
*/
    inline const Status MOVED_PERMANENTLY { 301, "Moved Permanently" };

    /*
   The 302 (Found) status code indicates that the target resource
   resides temporarily under a different URI.  Since the redirection
   might be altered on occasion, the client ought to continue to use the
   effective request URI for future requests.

   The server SHOULD generate a Location header field in the response
   containing a URI reference for the different URI.  The user agent MAY
   use the Location field value for automatic redirection.  The server's
   response payload usually contains a short hypertext note with a
   hyperlink to the different URI(s).

   Note: For historical reasons, a user agent MAY change the request
   method from POST to GET for the subsequent request.  If this
   behavior is undesired, the 307 (Temporary Redirect) status code
   can be used instead.

*/
    inline const Status FOUND { 302, "Found" };

    /*
   The 303 (See Other) status code indicates that the server is
   redirecting the user agent to a different resource, as indicated by a
   URI in the Location header field, which is intended to provide an
   indirect response to the original request.  A user agent can perform
   a retrieval request targeting that URI (a GET or HEAD request if
   using HTTP), which might also be redirected, and present the eventual
   result as an answer to the original request.  Note that the new URI
   in the Location header field is not considered equivalent to the
   effective request URI.

   This status code is applicable to any HTTP method.  It is primarily
   used to allow the output of a POST action to redirect the user agent
   to a selected resource, since doing so provides the information
   corresponding to the POST response in a form that can be separately
   identified, bookmarked, and cached, independent of the original
   request.

   A 303 response to a GET request indicates that the origin server does
   not have a representation of the target resource that can be
   transferred by the server over HTTP.  However, the Location field
   value refers to a resource that is descriptive of the target
   resource, such that making a retrieval request on that other resource
   might result in a representation that is useful to recipients without
   implying that it represents the original target resource.  Note that
   answers to the questions of what can be represented, what
   representations are adequate, and what might be a useful description
   are outside the scope of HTTP.

   Except for responses to a HEAD request, the representation of a 303
   response ought to contain a short hypertext note with a hyperlink to
   the same URI reference provided in the Location header field.
*/
    inline const Status SEE_OTHER { 303, "See Other" };

    /*
   The 305 (Use Proxy) status code was defined in a previous version of
   this specification and is now deprecated (Appendix B).
*/
    inline const Status USE_PROXY { 305, "Use Proxy" };

    /*
   The 306 status code was defined in a previous version of this
   specification, is no longer used, and the code is reserved.
*/
    inline const Status UNUSED { 306, "(Unused)" };

    /*

   The 307 (Temporary Redirect) status code indicates that the target
   resource resides temporarily under a different URI and the user agent
   MUST NOT change the request method if it performs an automatic
   redirection to that URI.  Since the redirection can change over time,
   the client ought to continue using the original effective request URI
   for future requests.

   The server SHOULD generate a Location header field in the response
   containing a URI reference for the different URI.  The user agent MAY
   use the Location field value for automatic redirection.  The server's
   response payload usually contains a short hypertext note with a
   hyperlink to the different URI(s).

   Note: This status code is similar to 302 (Found), except that it
   does not allow changing the request method from POST to GET.  This
   specification defines no equivalent counterpart for 301 (Moved
   Permanently) ([RFC7238], however, defines the status code 308
   (Permanent Redirect) for this purpose).
*/
    inline const Status TEMPORARY_REDIRECT { 307, "Temporary Redirect" };

    /*
   The 400 (Bad Request) status code indicates that the server cannot or
   will not process the request due to something that is perceived to be
   a client error (e.g., malformed request syntax, invalid request
   message framing, or deceptive request routing).
 */
    inline const Status BAD_REQUEST { 400, "Bad Request" };

    /*

*/
    inline const Status UNAUTHORIZED { 401, "Unauthorized" };

    /*
   The 402 (Payment Required) status code is reserved for future use.
*/
    inline const Status PAYMENT_REQUIRED { 402, "Payment Required" };

    /*
   The 403 (Forbidden) status code indicates that the server understood
   the request but refuses to authorize it.  A server that wishes to
   make public why the request has been forbidden can describe that
   reason in the response payload (if any).

   If authentication credentials were provided in the request, the
   server considers them insufficient to grant access.  The client
   SHOULD NOT automatically repeat the request with the same
   credentials.  The client MAY repeat the request with new or different
   credentials.  However, a request might be forbidden for reasons
   unrelated to the credentials.

   An origin server that wishes to "hide" the current existence of a
   forbidden target resource MAY instead respond with a status code of
   404 (Not Found).
*/
    inline const Status FORBIDDEN { 403, "Forbidden" };

    /*
   The 404 (Not Found) status code indicates that the origin server did
   not find a current representation for the target resource or is not
   willing to disclose that one exists.  A 404 status code does not
   indicate whether this lack of representation is temporary or
   permanent; the 410 (Gone) status code is preferred over 404 if the
   origin server knows, presumably through some configurable means, that
   the condition is likely to be permanent.

   A 404 response is cacheable by default; i.e., unless otherwise
   indicated by the method definition or explicit cache controls (see
   Section 4.2.2 of [RFC7234]).
*/
    inline const Status NOT_FOUND { 404, "Not Found" };

    /*
   The 405 (Method Not Allowed) status code indicates that the method
   received in the request-line is known by the origin server but not
   supported by the target resource.  The origin server MUST generate an
   Allow header field in a 405 response containing a list of the target
   resource's currently supported methods.

   A 405 response is cacheable by default; i.e., unless otherwise
   indicated by the method definition or explicit cache controls (see
   Section 4.2.2 of [RFC7234]).
*/
    inline const Status METHOD_NOT_ALLOWED { 405, "Method Not Allowed" };

    /*
   The 406 (Not Acceptable) status code indicates that the target
   resource does not have a current representation that would be
   acceptable to the user agent, according to the proactive negotiation
   header fields received in the request (Section 5.3), and the server
   is unwilling to supply a default representation.

   The server SHOULD generate a payload containing a list of available
   representation characteristics and corresponding resource identifiers
   from which the user or user agent can choose the one most
   appropriate.  A user agent MAY automatically select the most
   appropriate choice from that list.  However, this specification does
   not define any standard for such automatic selection, as described in
   Section 6.4.1.
*/
    inline const Status NOT_ACCEPTABLE { 406, "Not Acceptable" };

    /*
   The 408 (Request Timeout) status code indicates that the server did
   not receive a complete request message within the time that it was
   prepared to wait.  A server SHOULD send the "close" connection option
   (Section 6.1 of [RFC7230]) in the response, since 408 implies that
   the server has decided to close the connection rather than continue
   waiting.  If the client has an outstanding request in transit, the
   client MAY repeat that request on a new connection.
*/
    inline const Status REQUEST_TIMEOUT { 408, "Request Timeout" };

    /*
   The 409 (Conflict) status code indicates that the request could not
   be completed due to a conflict with the current state of the target
   resource.  This code is used in situations where the user might be
   able to resolve the conflict and resubmit the request.  The server
   SHOULD generate a payload that includes enough information for a user
   to recognize the source of the conflict.

   Conflicts are most likely to occur in response to a PUT request.  For
   example, if versioning were being used and the representation being
   PUT included changes to a resource that conflict with those made by
   an earlier (third-party) request, the origin server might use a 409
   response to indicate that it can't complete the request.  In this
   case, the response representation would likely contain information
   useful for merging the differences based on the revision history.
*/
    inline const Status CONFLICT { 409, "Conflict" };

    /*
   The 410 (Gone) status code indicates that access to the target
   resource is no longer available at the origin server and that this
   condition is likely to be permanent.  If the origin server does not
   know, or has no facility to determine, whether or not the condition
   is permanent, the status code 404 (Not Found) ought to be used
   instead.

   The 410 response is primarily intended to assist the task of web
   maintenance by notifying the recipient that the resource is
   intentionally unavailable and that the server owners desire that
   remote links to that resource be removed.  Such an event is common
   for limited-time, promotional services and for resources belonging to
   individuals no longer associated with the origin server's site.  It
   is not necessary to mark all permanently unavailable resources as
   "gone" or to keep the mark for any length of time -- that is left to
   the discretion of the server owner.

   A 410 response is cacheable by default; i.e., unless otherwise
   indicated by the method definition or explicit cache controls (see
   Section 4.2.2 of [RFC7234]).
 */
    inline const Status GONE { 410, "Gone" };

    /*
   The 411 (Length Required) status code indicates that the server
   refuses to accept the request without a defined Content-Length
   (Section 3.3.2 of [RFC7230]).  The client MAY repeat the request if
   it adds a valid Content-Length header field containing the length of
   the message body in the request message.
*/
    inline const Status LENGTH_REQUIRED { 411, "Length Required" };

    /*
   The 413 (Payload Too Large) status code indicates that the server is
   refusing to process a request because the request payload is larger
   than the server is willing or able to process.  The server MAY close
   the connection to prevent the client from continuing the request.

   If the condition is temporary, the server SHOULD generate a
   Retry-After header field to indicate that it is temporary and after
   what time the client MAY try again.
*/
    inline const Status PAYLOAD_TOO_LARGE { 413, "Payload Too Large" };

    /*
   The 414 (URI Too Long) status code indicates that the server is
   refusing to service the request because the request-target (Section
   5.3 of [RFC7230]) is longer than the server is willing to interpret.
   This rare condition is only likely to occur when a client has
   improperly converted a POST request to a GET request with long query
   information, when the client has descended into a "black hole" of
   redirection (e.g., a redirected URI prefix that points to a suffix of
   itself) or when the server is under attack by a client attempting to
   exploit potential security holes.

   A 414 response is cacheable by default; i.e., unless otherwise
   indicated by the method definition or explicit cache controls (see
   Section 4.2.2 of [RFC7234]).
*/
    inline const Status URI_TOO_LONG { 414, "URI Too Long" };

    /*
   The 415 (Unsupported Media Type) status code indicates that the
   origin server is refusing to service the request because the payload
   is in a format not supported by this method on the target resource.
   The format problem might be due to the request's indicated
   Content-Type or Content-Encoding, or as a result of inspecting the
   data directly.
*/
    inline const Status UNSUPPORTED_MEDIA_TYPE { 415, "Unsupported Media Type" };

    /*
   The 417 (Expectation Failed) status code indicates that the
   expectation given in the request's Expect header field
   (Section 5.1.1) could not be met by at least one of the inbound
   servers.
*/
    inline const Status EXPECTATION_FAILED { 417, "Expectation Failed" };

    /*
   Any attempt to brew coffee with a teapot should result in the error
   code "418 I'm a teapot". The resulting entity body MAY be short and
   stout.
*/
    inline const Status IM_A_TEAPOT { 418, "I'm a teapot" };

    /*
   The 426 (Upgrade Required) status code indicates that the server
   refuses to perform the request using the current protocol but might
   be willing to do so after the client upgrades to a different
   protocol.  The server MUST send an Upgrade header field in a 426
   response to indicate the required protocol(s) (Section 6.7 of
   [RFC7230]).

   Example:

     HTTP/1.1 426 Upgrade Required
     Upgrade: HTTP/3.0
     Connection: Upgrade
     Content-Length: 53
     Content-Type: text/plain

     This service requires use of the HTTP/3.0 protocol.

*/
    inline const Status UPGRADE_REQUIRED { 426, "Upgrade Required" };

    /*
The 431 status code indicates that the server is unwilling to process
   the request because its header fields are too large.  The request MAY
   be resubmitted after reducing the size of the request header fields.

   It can be used both when the set of request header fields in total is
   too large, and when a single header field is at fault.  In the latter
   case, the response representation SHOULD specify which header field
   was too large.

   For example:

   HTTP/1.1 431 Request Header Fields Too Large
   Content-Type: text/html

   <html>
      <head>
         <title>Request Header Fields Too Large</title>
      </head>
      <body>
         <h1>Request Header Fields Too Large</h1>
         <p>The "Example" header was too large.</p>
      </body>
   </html>

   Responses with the 431 status code MUST NOT be stored by a cache.
*/
    inline const Status REQUEST_HEADER_FIELDS_TOO_LARGE { 431, "Request Header Fields Too Large" };

    /*
   The 500 (Internal Server Error) status code indicates that the server
   encountered an unexpected condition that prevented it from fulfilling
   the request.
*/
    inline const Status INTERNAL_SERVER_ERROR { 500, "Internal Server Error" };

    /*
The 501 (Not Implemented) status code indicates that the server does
not support the functionality required to fulfill the request.  This
    is the appropriate response when the server does not recognize the
request method and is not capable of supporting it for any resource.

A 501 response is cacheable by default; i.e., unless otherwise
indicated by the method definition or explicit cache controls (see
Section 4.2.2 of [RFC7234]).
*/
    inline const Status NOT_IMPLEMENTED { 501, "Not Implemented" };

    /*
   The 502 (Bad Gateway) status code indicates that the server, while
   acting as a gateway or proxy, received an invalid response from an
   inbound server it accessed while attempting to fulfill the request.
*/
    inline const Status BAD_GATEWAY { 502, "Bad Gateway" };

    /*
   The 503 (Service Unavailable) status code indicates that the server
   is currently unable to handle the request due to a temporary overload
   or scheduled maintenance, which will likely be alleviated after some
   delay.  The server MAY send a Retry-After header field
   (Section 7.1.3) to suggest an appropriate amount of time for the
   client to wait before retrying the request.

      Note: The existence of the 503 status code does not imply that a
      server has to use it when becoming overloaded.  Some servers might
      simply refuse the connection.
*/
    inline const Status SERVICE_UNAVAILABLE { 503, "Service Unavailable" };

    /*
The 504 (Gateway Timeout) status code indicates that the server,
while acting as a gateway or proxy, did not receive a timely response
from an upstream server it needed to access in order to complete the
    request.
*/
    inline const Status GATEWAY_TIMEOUT { 504, "Gateway Timeout" };

    /*
   The 505 (HTTP Version Not Supported) status code indicates that the
   server does not support, or refuses to support, the major version of
   HTTP that was used in the request message.  The server is indicating
   that it is unable or unwilling to complete the request using the same
   major version as the client, as described in Section 2.6 of
   [RFC7230], other than with this error message.  The server SHOULD
   generate a representation for the 505 response that describes why
   that version is not supported and what other protocols are supported
   by that server.
*/
    inline const Status VERSION_NOT_SUPPORTED { 505, "HTTP Version Not Supported" };
}

}

auto operator<<(std::ostream& os, http::Status const& status) -> std::ostream&;

#endif //WEBSERV_HTTPSTATUS_HPP
