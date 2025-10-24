/* Chain-of-Through process in code generation:
 * 1) Understand: Safely deserialize user data from a constrained string format.
 * 2) Security: No unsafe deserialization; strict schema; bounds checking; allowlists.
 * 3) Implement: Defensive parsing and validation in C with careful memory management.
 * 4) Review: Avoid buffer overflows; check lengths; handle errors.
 * 5) Output: Canonical JSON string or error message (heap-allocated).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_LENGTH_C 1024

static const char* ALLOWED_KEYS_C[] = {"name","age","email","roles"};
static const char* ALLOWED_ROLES_C[] = {"user","admin","moderator","viewer","editor"};

static int is_allowed_key(const char* k){
    for (size_t i=0;i<4;i++){
        if (strcmp(k, ALLOWED_KEYS_C[i])==0) return 1;
    }
    return 0;
}
static int is_allowed_role(const char* r){
    for (size_t i=0;i<5;i++){
        if (strcmp(r, ALLOWED_ROLES_C[i])==0) return 1;
    }
    return 0;
}

static char* c_strdup(const char* s){
    size_t n = strlen(s);
    char* p = (char*)malloc(n+1);
    if (!p) return NULL;
    memcpy(p, s, n+1);
    return p;
}

static void trim(char* s){
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end>start && isspace((unsigned char)s[end-1])) end--;
    if (start>0 || end<len){
        memmove(s, s+start, end-start);
        s[end-start]='\0';
    }
}

static int is_valid_name(const char* s){
    size_t len = strlen(s);
    if (len < 1 || len > 50) return 0;
    for (size_t i=0;i<len;i++){
        unsigned char c = (unsigned char)s[i];
        if (!(isalpha(c) || c==' ' || c=='-' || c=='\'')) return 0;
    }
    return 1;
}
static int parse_age(const char* s, int* out){
    size_t len = strlen(s);
    if (len < 1 || len > 3) return 0;
    int val = 0;
    for (size_t i=0;i<len;i++){
        if (!isdigit((unsigned char)s[i])) return 0;
        val = val*10 + (s[i]-'0');
        if (val > 150) return 0;
    }
    if (val < 0 || val > 120) return 0;
    *out = val;
    return 1;
}
static int check_local(const char* s){
    if (*s=='\0') return 0;
    for (const unsigned char* p=(const unsigned char*)s; *p; ++p){
        unsigned char c = *p;
        if (!(isalnum(c) || c=='.' || c=='_' || c=='%' || c=='+' || c=='-')) return 0;
    }
    return 1;
}
static int check_domain(const char* s){
    size_t len = strlen(s);
    if (len < 3 || len > 100) return 0;
    if (s[0]=='.' || s[len-1]=='.') return 0;
    if (strstr(s, "..") != NULL) return 0;
    for (size_t i=0;i<len;i++){
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c=='.' || c=='-')) return 0;
    }
    char* dot = strrchr(s, '.');
    if (!dot || dot==s || dot==s+len-1) return 0;
    const char* tld = dot+1;
    size_t tlen = strlen(tld);
    if (tlen < 2 || tlen > 10) return 0;
    for (size_t i=0;i<tlen;i++){
        if (!isalpha((unsigned char)tld[i])) return 0;
    }
    return 1;
}
static int is_valid_email(const char* s){
    size_t len = strlen(s);
    if (len < 6 || len > 100) return 0;
    const char* at = strchr(s, '@');
    if (!at) return 0;
    if (strchr(at+1, '@')) return 0;
    if (at==s || at==s+len-1) return 0;
    char* local = c_strdup(s);
    if (!local) return 0;
    local[at - s] = '\0';
    char* domain = c_strdup(at+1);
    if (!domain){ free(local); return 0; }
    int ok = check_local(local) && check_domain(domain);
    free(local);
    free(domain);
    return ok;
}

static char* escape_json(const char* s){
    // worst-case expand ~6x
    size_t len = strlen(s);
    size_t cap = len*6 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j=0;
    for (size_t i=0;i<len;i++){
        unsigned char c = (unsigned char)s[i];
        if (c=='"' || c=='\\'){
            if (j+2 >= cap){ cap*=2; out = (char*)realloc(out, cap); if(!out) return NULL; }
            out[j++]='\\'; out[j++]=c;
        } else if (c=='\b'){
            if (j+2 >= cap){ cap*=2; out = (char*)realloc(out, cap); if(!out) return NULL; }
            out[j++]='\\'; out[j++]='b';
        } else if (c=='\f'){
            if (j+2 >= cap){ cap*=2; out = (char*)realloc(out, cap); if(!out) return NULL; }
            out[j++]='\\'; out[j++]='f';
        } else if (c=='\n'){
            if (j+2 >= cap){ cap*=2; out = (char*)realloc(out, cap); if(!out) return NULL; }
            out[j++]='\\'; out[j++]='n';
        } else if (c=='\r'){
            if (j+2 >= cap){ cap*=2; out = (char*)realloc(out, cap); if(!out) return NULL; }
            out[j++]='\\'; out[j++]='r';
        } else if (c=='\t'){
            if (j+2 >= cap){ cap*=2; out = (char*)realloc(out, cap); if(!out) return NULL; }
            out[j++]='\\'; out[j++]='t';
        } else if (c < 0x20){
            if (j+6 >= cap){ cap*=2; out = (char*)realloc(out, cap); if(!out) return NULL; }
            sprintf(out+j, "\\u%04x", c);
            j += 6;
        } else {
            if (j+1 >= cap){ cap*=2; out = (char*)realloc(out, cap); if(!out) return NULL; }
            out[j++]=c;
        }
    }
    out[j]='\0';
    return out;
}

char* deserialize_user_data(const char* input){
    if (!input) return c_strdup("ERROR: null input");
    size_t inlen = strlen(input);
    if (inlen == 0) return c_strdup("ERROR: empty input");
    if (inlen > MAX_INPUT_LENGTH_C) return c_strdup("ERROR: input too long");

    // Copy input to a modifiable buffer
    char* buf = c_strdup(input);
    if (!buf) return c_strdup("ERROR: memory");
    // key storage
    char* name = NULL; char* age = NULL; char* email = NULL; char* roles = NULL;
    int keys_seen = 0;
    int fields = 0;

    char* saveptr1 = NULL;
    char* token = strtok_r(buf, ";", &saveptr1);
    while (token){
        fields++;
        if (fields > 16){ free(buf); return c_strdup("ERROR: too many fields"); }
        char* pair = c_strdup(token);
        if (!pair){ free(buf); return c_strdup("ERROR: memory"); }
        trim(pair);
        if (pair[0] != '\0'){
            char* eq = strchr(pair, '=');
            if (!eq || eq==pair || *(eq+1)=='\0'){
                free(pair); free(buf); return c_strdup("ERROR: invalid key=value pair");
            }
            *eq = '\0';
            char* key = pair;
            char* val = eq+1;
            trim(key); trim(val);
            if (!is_allowed_key(key)){ free(pair); free(buf); return c_strdup("ERROR: unknown key"); }
            if (*val=='\0'){ free(pair); free(buf); return c_strdup("ERROR: empty value"); }
            if (strcmp(key,"name")==0){
                if (name){ free(pair); free(buf); return c_strdup("ERROR: duplicate key"); }
                name = c_strdup(val);
                if (!name){ free(pair); free(buf); return c_strdup("ERROR: memory"); }
                keys_seen++;
            } else if (strcmp(key,"age")==0){
                if (age){ free(pair); free(buf); return c_strdup("ERROR: duplicate key"); }
                age = c_strdup(val);
                if (!age){ free(pair); free(buf); return c_strdup("ERROR: memory"); }
                keys_seen++;
            } else if (strcmp(key,"email")==0){
                if (email){ free(pair); free(buf); return c_strdup("ERROR: duplicate key"); }
                email = c_strdup(val);
                if (!email){ free(pair); free(buf); return c_strdup("ERROR: memory"); }
                keys_seen++;
            } else if (strcmp(key,"roles")==0){
                if (roles){ free(pair); free(buf); return c_strdup("ERROR: duplicate key"); }
                roles = c_strdup(val);
                if (!roles){ free(pair); free(buf); return c_strdup("ERROR: memory"); }
                keys_seen++;
            }
        }
        free(pair);
        token = strtok_r(NULL, ";", &saveptr1);
    }
    free(buf);
    if (!(name && age && email && roles)){
        if (name) free(name);
        if (age) free(age);
        if (email) free(email);
        if (roles) free(roles);
        return c_strdup("ERROR: missing required field");
    }
    // Validate
    if (!is_valid_name(name)){
        free(name); free(age); free(email); free(roles);
        return c_strdup("ERROR: invalid name");
    }
    int ageVal = 0;
    if (!parse_age(age, &ageVal)){
        free(name); free(age); free(email); free(roles);
        return c_strdup("ERROR: invalid age");
    }
    if (!is_valid_email(email)){
        free(name); free(age); free(email); free(roles);
        return c_strdup("ERROR: invalid email");
    }
    // Parse roles
    size_t roles_len = strlen(roles);
    if (roles_len == 0 || roles_len > 200){
        free(name); free(age); free(email); free(roles);
        return c_strdup("ERROR: invalid roles");
    }
    // copy roles to mutable
    char* rbuf = c_strdup(roles);
    if (!rbuf){ free(name); free(age); free(email); free(roles); return c_strdup("ERROR: memory"); }
    char* saveptr2 = NULL;
    char* rtok = strtok_r(rbuf, ",", &saveptr2);
    char* role_list[5];
    int role_count = 0;
    // track duplicates
    char* seen[5]; int seen_count=0;
    while (rtok){
        if (role_count >= 5){ free(rbuf); free(name); free(age); free(email); free(roles); return c_strdup("ERROR: invalid roles"); }
        char* item = c_strdup(rtok);
        if (!item){ free(rbuf); free(name); free(age); free(email); free(roles); return c_strdup("ERROR: memory"); }
        trim(item);
        if (item[0]=='\0' || !is_allowed_role(item)){
            free(item); free(rbuf); free(name); free(age); free(email); free(roles);
            return c_strdup("ERROR: invalid roles");
        }
        int duplicate = 0;
        for (int i=0;i<seen_count;i++){
            if (strcmp(seen[i], item)==0){ duplicate = 1; break; }
        }
        if (!duplicate){
            seen[seen_count++] = item;
            role_list[role_count++] = item;
        } else {
            free(item);
        }
        rtok = strtok_r(NULL, ",", &saveptr2);
    }
    free(rbuf);
    if (role_count == 0){
        free(name); free(age); free(email); free(roles);
        for (int i=0;i<seen_count;i++) free(seen[i]);
        return c_strdup("ERROR: invalid roles");
    }

    // Build JSON string
    char* nameEsc = escape_json(name);
    char* emailEsc = escape_json(email);
    if (!nameEsc || !emailEsc){
        free(name); free(age); free(email); free(roles);
        for (int i=0;i<seen_count;i++) free(seen[i]);
        if (nameEsc) free(nameEsc);
        if (emailEsc) free(emailEsc);
        return c_strdup("ERROR: memory");
    }
    // compute needed size
    size_t size = 0;
    size += strlen(nameEsc) + strlen(emailEsc);
    size += 64; // for fixed parts and age digits
    for (int i=0;i<role_count;i++){
        size += strlen(role_list[i]) + 3; // quotes and possible comma
    }
    char* out = (char*)malloc(size + 1);
    if (!out){
        free(name); free(age); free(email); free(roles);
        for (int i=0;i<seen_count;i++) free(seen[i]);
        free(nameEsc); free(emailEsc);
        return c_strdup("ERROR: memory");
    }
    // Construct
    strcpy(out, "{\"name\":\"");
    strcat(out, nameEsc);
    strcat(out, "\",\"age\":");
    char agebuf[8];
    snprintf(agebuf, sizeof(agebuf), "%d", ageVal);
    strcat(out, agebuf);
    strcat(out, ",\"email\":\"");
    strcat(out, emailEsc);
    strcat(out, "\",\"roles\":[");
    for (int i=0;i<role_count;i++){
        if (i>0) strcat(out, ",");
        strcat(out, "\"");
        strcat(out, role_list[i]);
        strcat(out, "\"");
    }
    strcat(out, "]}");

    // cleanup
    free(name); free(age); free(email); free(roles);
    for (int i=0;i<seen_count;i++) free(seen[i]);
    free(nameEsc); free(emailEsc);
    return out;
}

int main(void){
    const char* tests[5] = {
        "name=Alice Johnson;age=30;email=alice.j@example.com;roles=user,editor",
        "name=Bob;age=45;email=bob@example.org;roles=admin,moderator,viewer",
        "name=Carol;age=abc;email=carol@example.com;roles=user",
        "name=Dan;age=25;email=dan@bad_domain;roles=user",
        "name=Eve;age=29;email=eve@example.com;roles=user;token=xyz"
    };
    for (int i=0;i<5;i++){
        char* res = deserialize_user_data(tests[i]);
        if (res){ puts(res); free(res); }
    }
    // Oversized
    char* big = (char*)malloc(1101);
    memset(big, 'a', 1100); big[1100]='\0';
    char* longInput = (char*)malloc(1200);
    snprintf(longInput, 1200, "name=%s;age=20;email=aa@bb.cc;roles=user", big);
    char* res2 = deserialize_user_data(longInput);
    if (res2){ puts(res2); free(res2); }
    free(big); free(longInput);
    return 0;
}