#include "zappka_api_processing.h"

float text_w, text_h;
float max_scroll;
extern float timer;
extern int Scene;
extern float startY;
extern float elapsed;
extern float endY;
extern Button buttonsy[100];
bool readingoferta = false;
bool offermachen = false;
bool categoryornah = false;
extern bool obrazekdone;
extern bool requestdone;
const char *id_tokenk = NULL;
const char *refreshtoken = NULL;
extern int selectioncodelol;
char zappsystr[64];
bool canredeem = false;
extern long response_code;
bool ofertanow = false;
bool przycskmachen = false;
extern bool loadingshit;
char *ploy_iden = NULL;
extern const char *nejmenmachen;
const char *userajd;
const char* secrettotpglobal;
extern FILE *fptr;
extern char combinedText[128]; 
extern C2D_TextBuf g_staticBuf;
extern bool youfuckedup;
extern C2D_Text g_staticText[100];
bool activated = false;
bool aktywacja_done = false;
bool json_done = false;
bool dawajploy = false;
bool zonefeeddone = false;
bool categoryfeeddone = false;
extern int tileCount;
extern char tileNames[100][256];  
extern C2D_Font font[0];
C2D_TextBuf kupon_text_Buf;
C2D_Text g_kuponText[100];
extern C2D_Image couponbutton, couponbutton_pressed, more_button;
extern C2D_Image gobackplz;
extern C2D_Image act_button, act_pressed;
extern C2D_Image deact_button, deact_pressed;
const char* kurwacursor;
char ploy_ids_bo_kurwa_reload[100][50];
char typy_bo_kurwa_reload[100][50];
const char* aftermachenkurw;
time_t czas_wygasniecia;

time_t dawaj_expire_time(const char *jwt) {
    if (!jwt) return 0;

    const char *first_dot = strchr(jwt, '.');
    if (!first_dot) return 0;

    const char *second_dot = strchr(first_dot + 1, '.');
    if (!second_dot) return 0;

    size_t payload_len = second_dot - (first_dot + 1);
    if (payload_len >= 1024) return 0; // prevent buffer overflow

    char payload_base64[1024];
    strncpy(payload_base64, first_dot + 1, payload_len);
    payload_base64[payload_len] = '\0';

    // Proper base64 padding
    size_t pad = payload_len % 4;
    if (pad == 2)
        strcat(payload_base64, "==");
    else if (pad == 3)
        strcat(payload_base64, "=");
    else if (pad == 1)
        return 0; // invalid base64

    // Base64 decode
    unsigned char decoded[2048];
    size_t decoded_len;
    int ret = mbedtls_base64_decode(decoded, sizeof(decoded), &decoded_len,
                                    (const unsigned char *)payload_base64, strlen(payload_base64));
    if (ret != 0) return 0;

    decoded[decoded_len] = '\0'; // make sure it's null-terminated for JSON

    json_error_t error;
    json_t *root = json_loads((const char *)decoded, 0, &error);
    if (!root) return 0;

    // "exp" might be in root or inside "data"
    json_t *exp_obj = json_object_get(root, "exp");
    if (!exp_obj) {
        json_t *data = json_object_get(root, "data");
        if (data) exp_obj = json_object_get(data, "exp");
    }

    time_t exp_time = 0;
    if (exp_obj && json_is_integer(exp_obj)) {
        exp_time = (time_t)json_integer_value(exp_obj);
    }

    json_decref(root);
    return exp_time;
}
time_t portable_timegm(struct tm *tm) {
    char *tz = getenv("TZ");
    setenv("TZ", "", 1); // Force UTC
    tzset();
    time_t ret = mktime(tm);
    if (tz) setenv("TZ", tz, 1); else unsetenv("TZ");
    tzset();
    return ret;
}
time_t snrs_czas() {
    struct curl_slist *headers = NULL;

    refresh_data("https://zabka-snrs.zabka.pl/v4/server/time", "", headers);

    const char *key = "\"serverTime\": \"";
    const char *start = strstr(global_response.data, key);
    if (!start) {
        printf("Error: serverTime key not found in response\n");
        return 0;
    }

    start += strlen(key);
    const char *end = strchr(start, '"');
    if (!end || (end - start >= 32)) {
        printf("Error: malformed serverTime format\n");
        return 0;
    }

    char time_str[32] = {0};
    strncpy(time_str, start, end - start);
    time_str[end - start] = '\0';

    // Expected format: "2025-06-17T14:49:24.384Z"
    struct tm t = {0};
    int millis = 0;
    int parsed = sscanf(time_str, "%4d-%2d-%2dT%2d:%2d:%2d.%3dZ",
                        &t.tm_year, &t.tm_mon, &t.tm_mday,
                        &t.tm_hour, &t.tm_min, &t.tm_sec, &millis);

    if (parsed < 6) {
        printf("Error: Failed to parse time string: %s\n", time_str);
        return 0;
    }

    t.tm_year -= 1900;
    t.tm_mon  -= 1;

    time_t unix_time = portable_timegm(&t);
    if (unix_time == (time_t)(-1)) {
        printf("Error: portable_timegm failed\n");
        return 0;
    }
	
    return unix_time;
}

void removeTrailingNewline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; 
    }
}
void wrapText(const char* input, int maxWidth, char* output) {
    int currentLineLength = 0;
    const char* wordStart = input;
    const char* wordEnd;
    char* outPtr = output;
    int firstWord = 1;

    while (*wordStart) {
        wordEnd = wordStart;
        while (*wordEnd && *wordEnd != ' ') wordEnd++;
        int wordLen = wordEnd - wordStart;

        if (currentLineLength + wordLen >= maxWidth) {
            if (outPtr != output)  
                *outPtr++ = '\n';
            currentLineLength = 0;
        }

        if (!firstWord) {
            *outPtr++ = ' ';
            currentLineLength++;
        } else {
            firstWord = 0;
        }

        strncpy(outPtr, wordStart, wordLen);
        outPtr += wordLen;
        currentLineLength += wordLen;

        wordStart = wordEnd;
        while (*wordStart == ' ') wordStart++;
    }

    *outPtr = '\0';  
}
void updateprom() {
	for (int i = 0; i < 30; i++) {
		if (ploy_ids_bo_kurwa_reload[i][0] != '\0') {
			log_to_file("%d: %s ", i, ploy_ids_bo_kurwa_reload[i]);
		}
	}
	log_to_file("\n");

	obrazekdone = false;
	requestdone = false;
	timer = 90.0f;
    Scene = 14;
	startY = 0.0f;
	elapsed = 0.0f;
	endY = -400.0f; 
	przycskmachen = false;
	loadingshit = true;
	categoryfeeddone = false;
	json_done = false;
	log_to_file("json_done: %d", json_done);
	log_to_file("loadingshit: %d", loadingshit);
	log_to_file("categoryfeeddone: %d", categoryfeeddone);
	log_to_file("zonefeeddone: %d", zonefeeddone);
	if (sfx->numChannels == 2) {
		cwavPlay(sfx, 0, 1);
	} else {
		cwavPlay(sfx, 0, -1);
	}

	removeButtonEntries(70);
	process_ids(selectioncodelol);
	update_promki(id_tokenk, refreshtoken);
	log_to_file("json_done: %d", json_done);
	log_to_file("loadingshit: %d", loadingshit);
	log_to_file("categoryfeeddone: %d", categoryfeeddone);
	log_to_file("zonefeeddone: %d", zonefeeddone);

}
void update_promki(const char* mejntoken, const char* refrenentokenenkurwen) {

	
    json_t *rootn = json_object();
    json_t *variablesn = json_object();
    if (!offermachen) {
		if (categoryornah) {
			json_object_set_new(rootn, "operationName", json_string("PloyOfferListing"));
			json_object_set_new(rootn, "query", json_string("query PloyOfferListing($ployOfferListingId: ID!, $after: String) { ployOfferListing(ployOfferListingId: $ployOfferListingId, after: $after) { __typename ...PloyOfferListingParts } }  fragment PriceParts on Price { amount currencyCode fractionDigits }  fragment ColorParts on ThemeColor { light dark }  fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } }  fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } }  fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } }  fragment HappyHourParts on HappyHour { alert validFrom validUntil }  fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } }  fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } }  fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename }  fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil }  fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename }  fragment PloyOfferParts on PloyOffer { id coupon { __typename ...CouponParts id } price { base discounted } visualDiscount __typename }  fragment PloyOfferListingParts on PloyOfferListing { id title ployOffers { __typename ...PloyOfferParts id } pagination { cursor } }"));
			json_object_set_new(rootn, "variables", variablesn);
			json_object_set_new(variablesn, "ployOfferListingId", json_string(ploy_iden));
		} else {
			json_object_set_new(rootn, "operationName", json_string("CategoryFeed"));
			json_object_set_new(rootn, "query", json_string("query CategoryFeed($categoryId: ID!, $after: String) { offerCategoryFeed(offerCategoryId: $categoryId, after: $after) { offerCategory { __typename ...OfferCategoryL1Parts } title sections { __typename ... on DealCarousel { __typename ...DealCarouselParts } ... on PloyOfferListingCarousel { __typename ...PloyOfferListingCarouselParts } ... on ProductPromotionListingCarousel { __typename ...ProductPromotionListingCarouselParts } ... on PromotionBannerCarousel { __typename ...PromotionBannerCarouselParts } ... on OptionTileSection { __typename ...OptionTileSectionParts } ... on StoriesSection { __typename ...StoriesSectionParts } } pagination { cursor } } } fragment OfferCategoryL1Parts on OfferCategory { id name levels { __typename ... on OfferCategory { id name } ... on PloyOfferListing { id title } ... on ProductPromotionListing { id title } } } fragment PriceParts on Price { amount currencyCode fractionDigits } fragment ColorParts on ThemeColor { light dark } fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } } fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff } fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff } fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } } fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } } fragment HappyHourParts on HappyHour { alert validFrom validUntil } fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } } fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } } fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename } fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil } fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil } fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil } fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename } fragment DealParts on Deal { id coupon { __typename ...CouponParts id } ployPrice __typename } fragment ScrollCarouselPropertiesParts on ScrollCarouselProperties { id title showMore } fragment DealCarouselParts on DealCarousel { id deals { __typename ...DealParts id } properties { __typename ...ScrollCarouselPropertiesParts } } fragment PloyOfferParts on PloyOffer { id coupon { __typename ...CouponParts id } price { base discounted } visualDiscount __typename } fragment PloyOfferListingParts on PloyOfferListing { id title ployOffers { __typename ...PloyOfferParts id } pagination { cursor } } fragment PloyOfferListingCarouselParts on PloyOfferListingCarousel { id properties { __typename ...ScrollCarouselPropertiesParts } ployOfferListing { __typename ...PloyOfferListingParts } } fragment ProductPromotionListingParts on ProductPromotionListing { id title productPromotions { __typename ...ProductPromotionParts id } pagination { cursor } } fragment ProductPromotionListingCarouselParts on ProductPromotionListingCarousel { id title properties { __typename ...ScrollCarouselPropertiesParts } productPromotionListing { __typename ...ProductPromotionListingParts } } fragment SwipeCarouselPropertiesParts on SwipeCarouselProperties { swipeId: id swipeTitle: title } fragment AutoplayCarouselPropertiesParts on AutoplayCarouselProperties { autoplayId: id autoplayTitle: title autoplayIntervalMillis } fragment LargePromotionBannerPropertiesParts on LargePromotionBannerProperties { largeTitle: title largeSubtitle: subtitle image { url } } fragment MediumPromotionBannerPropertiesParts on MediumPromotionBannerProperties { mediumTitle: title mediumSubtitle: subtitle image { url } } fragment CtaParts on Cta { title subtitle action { label link } id image { url } __typename } fragment QuestParts on Quest { id title subtitle bannerImage { url } detailsImage { url } lastingAt description questCompleted repeatable badges progressSummary { status totalSteps finishedSteps } stages { status subStages { status description totalSteps finishedSteps } completionDescription } } fragment PromotionBannerCarouselParts on PromotionBannerCarousel { id title carouselProperties: properties { __typename ... on SwipeCarouselProperties { __typename ...SwipeCarouselPropertiesParts } ... on AutoplayCarouselProperties { __typename ...AutoplayCarouselPropertiesParts } } banners { __typename ... on ProductPromotionBanner { id productPromotion { __typename ...ProductPromotionParts id } properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } } ... on CtaBanner { id cta { __typename ...CtaParts id } properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } header label } ... on Deal { __typename ...DealParts id } ... on LinkBanner { id header link properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } } ... on Quest { __typename ...QuestParts } } } fragment OptionTileParts on OptionTile { id label link } fragment OptionTileSectionParts on OptionTileSection { id title subtitle optionTiles { __typename ...OptionTileParts } } fragment StoriesSectionParts on StoriesSection { id title storiesSource labels }"));
			json_object_set_new(rootn, "variables", variablesn);
			json_object_set_new(variablesn, "categoryId", json_string(ploy_iden));
		}
	} else {
		json_object_set_new(rootn, "operationName", json_string("PloyOfferDetails"));
		json_object_set_new(rootn, "query", json_string("query PloyOfferDetails($ployOfferId: ID!) { ployOffer(ployOfferId: $ployOfferId) { ployOffer { __typename ...PloyOfferParts id } details { __typename ...RichTextWithHeaderParts } } }  fragment PriceParts on Price { amount currencyCode fractionDigits }  fragment ColorParts on ThemeColor { light dark }  fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } }  fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } }  fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } }  fragment HappyHourParts on HappyHour { alert validFrom validUntil }  fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } }  fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } }  fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename }  fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil }  fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename }  fragment PloyOfferParts on PloyOffer { id coupon { __typename ...CouponParts id } price { base discounted } visualDiscount __typename }  fragment RichTextWithHeaderParts on RichTextWithHeader { header richText }"));
		json_object_set_new(rootn, "variables", variablesn);
		json_object_set_new(variablesn, "ployOfferId", json_string(ploy_iden));
	}

    char *json_datan = json_dumps(rootn, JSON_COMPACT);
    json_decref(rootn);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");

    headers = curl_slist_append(headers, "Accept: application/json");

    char auth_headeren[1024];
    snprintf(auth_headeren, sizeof(auth_headeren), "Authorization: Bearer %s", mejntoken);
    headers = curl_slist_append(headers, auth_headeren);

    LightEvent response_event;
    LightEvent_Init(&response_event, RESET_ONESHOT);

    char *local_response = NULL;
	size_t response_size = 0;

    log_to_file("Queuing request...");

    queue_request("https://api.spapp.zabka.pl/", json_datan, headers, (void **)&local_response, &response_size, &response_event, false);

}
void load_m_prom() {
	if (sfx->numChannels == 2) {
		cwavPlay(sfx, 0, 1);
	} else {
		cwavPlay(sfx, 0, -1);
	}

	loadmore_promki(id_tokenk, refreshtoken, kurwacursor, aftermachenkurw);

}
void loadmore_promki(const char* mejntoken, const char* refrenentokenenkurwen, const char* aftermachen, const char* afterajd) {
	log_to_file("\n aftermachen: %s", aftermachen);
	log_to_file("\n afterajd: %s", afterajd);

	
    json_t *rootn = json_object();
    json_t *variablesn = json_object();
	json_object_set_new(rootn, "operationName", json_string("PloyOfferListing"));
	json_object_set_new(rootn, "query", json_string("query PloyOfferListing($ployOfferListingId: ID!, $after: String) { ployOfferListing(ployOfferListingId: $ployOfferListingId, after: $after) { __typename ...PloyOfferListingParts } }  fragment PriceParts on Price { amount currencyCode fractionDigits }  fragment ColorParts on ThemeColor { light dark }  fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } }  fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } }  fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } }  fragment HappyHourParts on HappyHour { alert validFrom validUntil }  fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } }  fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } }  fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename }  fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil }  fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename }  fragment PloyOfferParts on PloyOffer { id coupon { __typename ...CouponParts id } price { base discounted } visualDiscount __typename }  fragment PloyOfferListingParts on PloyOfferListing { id title ployOffers { __typename ...PloyOfferParts id } pagination { cursor } }"));
	json_object_set_new(rootn, "variables", variablesn);
	json_object_set_new(variablesn, "ployOfferListingId", json_string(afterajd));
	json_object_set_new(variablesn, "after", json_string(aftermachen));

    char *json_datan = json_dumps(rootn, JSON_COMPACT);
    json_decref(rootn);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");

    headers = curl_slist_append(headers, "Accept: application/json");

    char auth_headeren[1024];
    snprintf(auth_headeren, sizeof(auth_headeren), "Authorization: Bearer %s", mejntoken);
    headers = curl_slist_append(headers, auth_headeren);

    refresh_data("https://api.spapp.zabka.pl/", json_datan, headers);
    log_to_file("Processing JSON response...");
	bool cursor_here = false;
    json_t *response_root = json_loads(global_response.data, 0, NULL);
	json_t *data = json_object_get(response_root, "data");
	json_t *ployOfferListing = json_object_get(data, "ployOfferListing");
	offermachen = true;
	json_t *ployOffers = json_object_get(ployOfferListing, "ployOffers");
	json_t *pag = json_object_get(ployOfferListing, "pagination");
	json_t *cur = json_object_get(pag, "cursor");
	if (cur && json_typeof(cur) == JSON_STRING) {
		kurwacursor = strdup(json_string_value(cur));
		cursor_here = true;
	}

	int kurwavar = 0;
	kurwavar = tileCount;
	log_to_file("tlc: %d\n", tileCount);
	log_to_file("kurv: %d\n", kurwavar);
	tileCount = 0;
	size_t index;
	json_t *tile;
	
	json_array_foreach(ployOffers, index, tile) {
		if (tileCount >= json_array_size(ployOffers)) break; 
		json_t *first_section = json_array_get(ployOffers, index);  


		json_t *id = json_object_get(first_section, "id");
		json_t *typename = json_object_get(first_section, "__typename");
		const char *type_st = json_string_value(typename);
		log_to_file("typkurwa_%d: %s", tileCount + kurwavar - 5, type_st);
		const char *id_st = json_string_value(id);
		log_to_file("idkurwa_%d: %s", tileCount + kurwavar - 5, id_st);
		strncpy(typy_bo_kurwa_reload[tileCount + kurwavar - 5], type_st, 49);
		typy_bo_kurwa_reload[tileCount + kurwavar - 5][49] = '\0'; 
		strncpy(ploy_ids_bo_kurwa_reload[tileCount + kurwavar - 5], id_st, 49);
		ploy_ids_bo_kurwa_reload[tileCount + kurwavar - 5][49] = '\0';  

		json_t *coupon = json_object_get(tile, "coupon");
		json_t *targetPromotion = json_object_get(coupon, "targetPromotion");
		json_t *name = json_object_get(targetPromotion, "name");

		const char *textenen = json_string_value(name);
		char texten[15];
		strncpy(texten, textenen, 14);
		texten[14] = '\0';

		if (texten) {

			C2D_TextFontParse(&g_kuponText[tileCount + kurwavar - 5], font[0], kupon_text_Buf, texten);
			C2D_TextOptimize(&g_kuponText[tileCount + kurwavar - 5]);
			log_to_file("adding button: %d \n", tileCount + kurwavar + 1);
			buttonsy[tileCount + kurwavar] = (Button){
				.x = 0,
				.y = 0,
				.width = 134,
				.height = 179,
				.imageNormal = couponbutton,
				.imagePressed = couponbutton_pressed,
				.isPressed = false,
				.scene = 14,
				.scene2 = 15,
				.scene3 = 15,
				.scene4 = 15,
				.scene5 = 15,
				.sizenmachen = 1.0f,
				.onClick = updateprom
			};

			if (texten) {
				strncpy(tileNames[tileCount + kurwavar], texten, sizeof(tileNames[tileCount + kurwavar]) - 1);
				tileNames[tileCount + kurwavar][sizeof(tileNames[tileCount + kurwavar]) - 1] = '\0';
			}

			log_to_file("Tile %zu: %s", index, tileNames[tileCount + kurwavar]);
			log_to_file("Button %zu", tileCount + kurwavar);
			max_scroll += buttonsy[5 + tileCount].width + 8;
			tileCount++;
		
		}
	}
	if (cursor_here) {
		buttonsy[tileCount + kurwavar] = (Button){0, 0, 134, 179, more_button, more_button, false, 14, 15, 15, 15, 15, 1.0f, load_m_prom};
		max_scroll += buttonsy[tileCount + kurwavar].width + 8;
	}
	max_scroll -= 134;
	tileCount = tileCount + kurwavar; 
    json_decref(response_root);


}
void activate_coupon(const char* mejntoken, const char* refrenentokenenkurwen) {

	
    json_t *rootn = json_object();
    json_t *variablesn = json_object();
	json_t *activateCouponInputn = json_object();
    if (activated) {
		json_object_set_new(rootn, "operationName", json_string("ActivateCoupon"));
		json_object_set_new(rootn, "query", json_string("mutation ActivateCoupon($activateCouponInput: ActivateCouponInput!) { activateCoupon(activateCouponInput: $activateCouponInput) { __typename } }"));
		json_object_set_new(rootn, "variables", variablesn);
		json_object_set_new(variablesn, "activateCouponInput", activateCouponInputn);
		json_object_set_new(activateCouponInputn, "amount", json_integer(1));
		json_object_set_new(activateCouponInputn, "couponId", json_string(ploy_iden));
	} else {
		json_object_set_new(rootn, "operationName", json_string("DeactivateCoupon"));
		json_object_set_new(rootn, "query", json_string("mutation DeactivateCoupon($input: DeactivateCouponInput!) { deactivateCoupon(deactivateCouponInput: $input) { __typename } }"));
		json_object_set_new(rootn, "variables", variablesn);
		json_object_set_new(variablesn, "input", activateCouponInputn);
		json_object_set_new(activateCouponInputn, "couponId", json_string(ploy_iden));
	}

    char *json_datan = json_dumps(rootn, JSON_COMPACT);
    json_decref(rootn);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");

    headers = curl_slist_append(headers, "Accept: application/json");

    char auth_headeren[1024];
    snprintf(auth_headeren, sizeof(auth_headeren), "Authorization: Bearer %s", mejntoken);
    headers = curl_slist_append(headers, auth_headeren);

    LightEvent response_event;
    LightEvent_Init(&response_event, RESET_ONESHOT);

    char *local_response = NULL;
	size_t response_size = 0;

    log_to_file("Queuing request...");
    queue_request("https://api.spapp.zabka.pl/", json_datan, headers, (void **)&local_response, &response_size, &response_event, false);
	aktywacja_done = true;
	buttonsy[4] = (Button){0};
}
void process_ids(int selectedbutton) {
	LightLock_Lock(&global_response_lock);
	log_to_file("Selected Button: %d", selectedbutton);
	json_done = false;
	json_t *response_root = json_loads(global_response.data, 0, NULL);

	json_t *data = json_object_get(response_root, "data");
	
	if (!dawajploy) {
		if (!offermachen) {
			json_t *zoneFeed = json_object_get(data, "zoneFeed");


			json_t *sections = json_object_get(zoneFeed, "sections");


			json_t *first_section = json_array_get(sections, 0);


			json_t *tiles = json_object_get(first_section, "tiles");	
			log_to_file("button: %d", selectedbutton);
			size_t tile_count = json_array_size(tiles);
			log_to_file("Number of tiles: %zu", tile_count);
			json_t *tilesselected = json_array_get(tiles, selectedbutton - 4);
			log_to_file("arraytle: %d", selectedbutton - 4);
			json_t *offerTreeLevel = json_object_get(tilesselected, "offerTreeLevel");
			json_t *id = json_object_get(offerTreeLevel, "id");
			json_t *typename = json_object_get(offerTreeLevel, "__typename");
			if (strcmp(json_string_value(typename), "OfferCategory")) {
				categoryornah = true;
			} else {
				categoryornah = false;
			}

			if (id) {
				free(ploy_iden);
				ploy_iden = strdup(json_string_value(id));
				log_to_file("\nLoading id: %s...", ploy_iden);
				aftermachenkurw = strdup(ploy_iden);
			}
			dawajploy = true;
			log_to_file("zone not cat");
		} else {
			json_t *ployOfferListing = json_object_get(data, "ployOfferListing");


			json_t *ployOffers = json_object_get(ployOfferListing, "ployOffers");


			json_t *first_section = json_array_get(ployOffers, selectedbutton - 5);


			if (categoryfeeddone) {
				if (strcmp(typy_bo_kurwa_reload[selectedbutton - 5], "PloyOffer")) {
					dawajploy = false;
				} else {
					offermachen = false;
				}
			}
			if (ploy_ids_bo_kurwa_reload[selectedbutton - 5]) {
				free(ploy_iden);
				ploy_iden = strdup(ploy_ids_bo_kurwa_reload[selectedbutton - 5]);
				log_to_file("\nLoading id: %s...", ploy_iden);
				log_to_file("\nploy: %d", selectedbutton - 5);
				aftermachenkurw = strdup(ploy_iden);
			}
		}
	} else if (dawajploy) {
		if (offermachen) {
			json_t *zoneFeed = json_object_get(data, "ployOfferListing");


			json_t *sections = json_object_get(zoneFeed, "ployOffers");
			json_t *tilesselected = json_array_get(sections, selectedbutton - 5);
			json_t *id = json_object_get(tilesselected, "id");
			if (ploy_ids_bo_kurwa_reload[selectedbutton - 5]) {
				free(ploy_iden);
				ploy_iden = strdup(ploy_ids_bo_kurwa_reload[selectedbutton - 5]);
				log_to_file("\nLoading id: %s...", ploy_iden);
			}
			log_to_file("oferta");
		} else {
			json_t *offerCategoryFeed = json_object_get(data, "offerCategoryFeed");


			json_t *offerCategory = json_object_get(offerCategoryFeed, "offerCategory");
			json_t *levels = json_object_get(offerCategory, "levels");
			log_to_file("button: %d", selectedbutton);
			size_t tile_count = json_array_size(levels);
			log_to_file("Number of tiles: %zu", tile_count);
			json_t *tilesselected = json_array_get(levels, selectedbutton - 5);
			log_to_file("arraytle: %d", selectedbutton - 5);
			json_t *id = json_object_get(tilesselected, "id");
			json_t *typename = json_object_get(tilesselected, "__typename");
			if (strcmp(json_string_value(typename), "OfferCategory")) {
				categoryornah = true;
			} else {
				categoryornah = false;
			}
			if (id) {
				free(ploy_iden);
				ploy_iden = strdup(json_string_value(id));
				log_to_file("\nLoading id: %s...", ploy_iden);
				aftermachenkurw = strdup(ploy_iden);
			}
			dawajploy = false;
			log_to_file("catfeed not cat");
		}
	} 
	json_decref(response_root);
	log_to_file("id: %s", ploy_iden);
	LightLock_Unlock(&global_response_lock);

}
void process_kupony() {
	memset(ploy_ids_bo_kurwa_reload, 0, sizeof(ploy_ids_bo_kurwa_reload));
	log_to_file("Processing Kupony...");
	log_to_file("json_done: %d", json_done);
	log_to_file("loadingshit: %d", loadingshit);
	log_to_file("categoryfeeddone: %d", categoryfeeddone);
	log_to_file("zonefeeddone: %d", zonefeeddone);
	printf("Processing Kupony...\n");
	LightLock_Lock(&global_response_lock);
    if (global_response.data) {
        log_to_file("Processing JSON response...");

        json_t *response_root = json_loads(global_response.data, 0, NULL);
        if (!response_root) {
            log_to_file("ERROR: Failed to parse JSON response.");
			printf("\nERROR: Failed to parse JSON response.");
            free_global_response();

            return;
        }

        json_t *data = json_object_get(response_root, "data");
        if (!data) {
            log_to_file("ERROR: 'data' not found in the response.");
            json_decref(response_root);
            return;
        }

        json_t *zoneFeed = json_object_get(data, "zoneFeed");
        if (!zoneFeed) {
            log_to_file("ERROR: 'zoneFeed' not found in the data.");
            json_decref(response_root);
            return;
        }

        json_t *sections = json_object_get(zoneFeed, "sections");
        if (!sections || !json_is_array(sections)) {
            log_to_file("ERROR: 'sections' not found or not an array.");
            json_decref(response_root);
            return;
        }

        json_t *first_section = json_array_get(sections, 0);
        if (!first_section) {
            log_to_file("ERROR: 'sections[0]' is null.");
            json_decref(response_root);
            return;
        }

        json_t *tiles = json_object_get(first_section, "tiles");
        if (!tiles || !json_is_array(tiles)) {
            log_to_file("ERROR: 'tiles' not found or not an array.");
            json_decref(response_root);
            return;
        }

        tileCount = 0;
        size_t index;
        json_t *tile;
		max_scroll = 0;
		json_array_foreach(tiles, index, tile) {
			if (tileCount >= json_array_size(tiles)) break;

			json_t *offerTreeLevel = json_object_get(tile, "offerTreeLevel");
			if (offerTreeLevel) {
				json_t *title = json_object_get(offerTreeLevel, "title");
				json_t *name = json_object_get(offerTreeLevel, "name");
				if (strcmp(json_string_value(title) ? json_string_value(title) : json_string_value(name), "Partnerzy")) {
					const char *text = json_string_value(title) ? json_string_value(title) : json_string_value(name);
					if (text) {

						C2D_TextFontParse(&g_kuponText[tileCount], font[0], kupon_text_Buf, text);
						C2D_TextOptimize(&g_kuponText[tileCount]);
						
						buttonsy[5 + tileCount] = (Button){
							.x = 0,
							.y = 0,
							.width = 134,
							.height = 179,
							.imageNormal = couponbutton,
							.imagePressed = couponbutton_pressed,
							.isPressed = false,
							.scene = 14,
							.scene2 = 15,
							.scene3 = 15,
							.scene4 = 15,
							.scene5 = 15,
							.sizenmachen = 1.0f,
							.onClick = updateprom
						};

						if (text) {
							strncpy(tileNames[tileCount], text, sizeof(tileNames[tileCount]) - 1);
							tileNames[tileCount][sizeof(tileNames[tileCount]) - 1] = '\0';
						}
						log_to_file("Tile %zu: %s", index, tileNames[tileCount]);
						printf("\nTile %zu: %s", index, tileNames[tileCount]);
						log_to_file("Button %zu", 5 + tileCount);
						printf("\nButton %zu", 5 + tileCount);
						max_scroll += buttonsy[5 + tileCount].width + 8;
						tileCount++;
					}
				}
			}
		}
		max_scroll -= 134;
        json_decref(response_root);
		zonefeeddone = true;
		loadingshit = true;

    } else {
        log_to_file("ERROR: No response data available to process.");
    }
	przycskmachen = true;
	log_to_file("json_done: %d", json_done);
	log_to_file("loadingshit: %d", loadingshit);
	log_to_file("categoryfeeddone: %d", categoryfeeddone);
	log_to_file("zonefeeddone: %d", zonefeeddone);
	LightLock_Unlock(&global_response_lock);
}
void process_category() {
	int kurwavar = 4;
    C2D_TextBufClear(kupon_text_Buf);
    
    log_to_file("Processing JSON response...");

    json_t *response_root = json_loads(global_response.data, 0, NULL);
    if (!response_root) {
        log_to_file("ERROR: Failed to parse JSON response.");
        free_global_response();

        return;
    }
	if (!categoryornah) {
		json_t *data = json_object_get(response_root, "data");
		json_t *offerCategoryFeed = json_object_get(data, "offerCategoryFeed");
		json_t *offerCategory = json_object_get(offerCategoryFeed, "offerCategory");
		json_t *levels = json_object_get(offerCategory, "levels");

		tileCount = 0;
		size_t index;
		json_t *tile;
		max_scroll = 0;
		json_array_foreach(levels, index, tile) {
			if (tileCount >= json_array_size(levels)) break;

			json_t *title = json_object_get(tile, "title");
			const char *text = json_string_value(title);

			if (text) {

				C2D_TextFontParse(&g_kuponText[tileCount], font[0], kupon_text_Buf, text);
				C2D_TextOptimize(&g_kuponText[tileCount]);

				buttonsy[5 + tileCount] = (Button){
					.x = 0,
					.y = 0,
					.width = 134,
					.height = 179,
					.imageNormal = couponbutton,
					.imagePressed = couponbutton_pressed,
					.isPressed = false,
					.scene = 14,
					.scene2 = 15,
					.scene3 = 15,
					.scene4 = 15,
					.scene5 = 15,
					.sizenmachen = 1.0f,
					.onClick = updateprom
				};
				if (text) {
					strncpy(tileNames[tileCount], text, sizeof(tileNames[tileCount]) - 1);
					tileNames[tileCount][sizeof(tileNames[tileCount]) - 1] = '\0';
				}

				log_to_file("Tile %zu: %s", index, tileNames[tileCount]);
				log_to_file("Button %zu", 5 + tileCount);
				max_scroll += buttonsy[5 + tileCount].width + 8;
				tileCount++;
			}
		}
	} else {
		bool cursor_here = false;
		json_t *data = json_object_get(response_root, "data");
		json_t *ployOfferListing = json_object_get(data, "ployOfferListing");
		offermachen = true;
		json_t *ployOffers = json_object_get(ployOfferListing, "ployOffers");
		json_t *pag = json_object_get(ployOfferListing, "pagination");
		json_t *cur = json_object_get(pag, "cursor");
		if (cur && json_typeof(cur) == JSON_STRING) {
			kurwacursor = strdup(json_string_value(cur));
			cursor_here = true;
		}



		tileCount = 0;
		size_t index;
		json_t *tile;
		max_scroll = 0;
		json_array_foreach(ployOffers, index, tile) {
			if (tileCount >= json_array_size(ployOffers)) break;
			json_t *first_section = json_array_get(ployOffers, index);


			json_t *id = json_object_get(first_section, "id");
			json_t *typename = json_object_get(first_section, "__typename");
			const char *type_st = json_string_value(typename);
			const char *id_st = json_string_value(id);
			strncpy(typy_bo_kurwa_reload[tileCount], type_st, 49);
			typy_bo_kurwa_reload[tileCount][49] = '\0';
			strncpy(ploy_ids_bo_kurwa_reload[tileCount], id_st, 49);
			ploy_ids_bo_kurwa_reload[tileCount][49] = '\0';
			log_to_file("id%d: %s\n", tileCount, id_st);
			json_t *coupon = json_object_get(tile, "coupon");
			json_t *targetPromotion = json_object_get(coupon, "targetPromotion");
			json_t *name = json_object_get(targetPromotion, "name");
			const char *textenen = json_string_value(name);
			char texten[15];
			strncpy(texten, textenen, 14);
			texten[14] = '\0';

			if (texten) {

				C2D_TextFontParse(&g_kuponText[tileCount], font[0], kupon_text_Buf, texten);
				C2D_TextOptimize(&g_kuponText[tileCount]);

				buttonsy[5 + tileCount] = (Button){
					.x = 0,
					.y = 0,
					.width = 134,
					.height = 179,
					.imageNormal = couponbutton,
					.imagePressed = couponbutton_pressed,
					.isPressed = false,
					.scene = 14,
					.scene2 = 15,
					.scene3 = 15,
					.scene4 = 15,
					.scene5 = 15,
					.sizenmachen = 1.0f,
					.onClick = updateprom
				};

				if (texten) {
					strncpy(tileNames[tileCount], texten, sizeof(tileNames[tileCount]) - 1);
					tileNames[tileCount][sizeof(tileNames[tileCount]) - 1] = '\0';
				}

				log_to_file("Tile %zu: %s", index, tileNames[tileCount]);
				log_to_file("Button %zu", 5 + tileCount);
				max_scroll += buttonsy[5 + tileCount].width + 8;
				tileCount++;
				kurwavar++;
			}
		}
		if (cursor_here){
			buttonsy[kurwavar + 1] = (Button){0, 0, 134, 179, more_button, more_button, false, 14, 15, 15, 15, 15, 1.0f, load_m_prom}; 
			max_scroll += buttonsy[kurwavar + 1].width + 8;
		}
		tileCount = tileCount + 5;
	}
	max_scroll -= 134;
    json_decref(response_root);

    przycskmachen = true;
	loadingshit = false;
	json_done = true;
	categoryfeeddone = false;
}
void aktywujkurwe() {
	activate_coupon(id_tokenk, refreshtoken);
}
void process_ofertamachen() {
	aktywacja_done = false;
    C2D_TextBufClear(kupon_text_Buf);
    
    log_to_file("Processing JSON response...");

    json_t *response_root = json_loads(global_response.data, 0, NULL);
    if (!response_root) {
        log_to_file("ERROR: Failed to parse JSON response.");
        free_global_response(); 

        return;
    }

    json_t *data = json_object_get(response_root, "data");
    json_t *ployOffer = json_object_get(data, "ployOffer");
	json_t *details = json_object_get(ployOffer, "details");
	json_t *first_item = json_array_get(details, 0);
	json_t *richText = json_object_get(first_item, "richText");
	const char *opisen = json_string_value(richText);
	json_t *ployOffer2 = json_object_get(ployOffer, "ployOffer");
	json_t *coupon = json_object_get(ployOffer2, "coupon");
	json_t *activate = json_object_get(coupon, "state");
	if (strcmp(json_string_value(activate), "ACTIVATED")) {
		activated = true;
		json_t *ajdyn = json_object_get(coupon, "id");
		ploy_iden = strdup(json_string_value(ajdyn));
	} else {
		activated = false;
		json_t *ajdyn = json_object_get(coupon, "id");
		ploy_iden = strdup(json_string_value(ajdyn));
	}
	json_t *targetPromotion = json_object_get(coupon, "targetPromotion");
	json_t *image = json_object_get(targetPromotion, "image");
	json_t *url = json_object_get(image, "url");
	const char *url_string = json_string_value(url);
	if (!url_string) {
		log_to_file("ERROR: 'url' is NULL or not a string!");
		json_decref(response_root);
		return;
	}

	char *urlimage = strdup(url_string);
	if (!urlimage) {
		log_to_file("ERROR: Failed to strdup url string.");
		json_decref(response_root);
		return;
	}
	log_to_file("\n %s", urlimage);
	char new_url[256]; 
	json_t *price = json_object_get(ployOffer2, "price");
	json_t *zappsyvalue = json_object_get(price, "base");
	int cena = json_integer_value(zappsyvalue);
	int cenamoja = atoi(zappsystr);
	log_to_file("mojeżapps: %d",cenamoja);
	if (cena <= cenamoja) {
		canredeem = true;
	} else {
		canredeem = false;
	}
	char cenastr[64];
	sprintf(cenastr, "%d Żappsów", cena);
	char wrapped[512];
	removeTrailingNewline(opisen);
    wrapText(opisen, 50, wrapped);  

    C2D_TextFontParse(&g_kuponText[0], font[0], kupon_text_Buf, wrapped);
    C2D_TextOptimize(&g_kuponText[0]);
    C2D_TextFontParse(&g_kuponText[1], font[0], kupon_text_Buf, cenastr);
    C2D_TextOptimize(&g_kuponText[1]);
	C2D_TextGetDimensions(&g_kuponText[0], 0.6f, 0.6f, &text_w, &text_h);
	max_scroll = text_h;
    json_decref(response_root);

    przycskmachen = true;
    categoryfeeddone = true;
	loadingshit = false;
	ofertanow = true;
	json_done = true;
	if (canredeem) {
		if (activated) {
			buttonsy[4] = (Button){40, 140, 243, 61, act_button, act_pressed, false, 14, 15, 15, 15, 15, 1.0f, aktywujkurwe}; 
		} else {
			buttonsy[4] = (Button){40, 140, 243, 61, deact_button, deact_pressed, false, 14, 15, 15, 15, 15, 1.0f, aktywujkurwe}; 
		}
	}
    json_t *rootn = json_object();
    json_object_set_new(rootn, "url", json_string(urlimage));

    char *json_datan = json_dumps(rootn, JSON_COMPACT);
    json_decref(rootn);
	free((void *)urlimage);

    LightEvent response_event;
    LightEvent_Init(&response_event, RESET_ONESHOT);
	uint8_t *local_response = NULL;
	size_t response_size = 0;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Id: a531998ec966db0951239efb91519560346cfecac77459fe3b85c5b786fa41de");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Name: LoyaltyPoints");
    headers = curl_slist_append(headers, "Accept: application/json");
	log_to_file("Raw JSON dump:\n%s", json_datan);
	queue_request("https://api.szprink.xyz/t3x/convert", json_datan, headers, (void **)&local_response, &response_size, &response_event, true);
	free(json_datan);
}
void process_ployoffers() {
    C2D_TextBufClear(kupon_text_Buf);
    
    log_to_file("Processing JSON response...");

    json_t *response_root = json_loads(global_response.data, 0, NULL);
    if (!response_root) {
        log_to_file("ERROR: Failed to parse JSON response.");
        free_global_response();

        return;
    }

    json_t *data = json_object_get(response_root, "data");
    json_t *ployOfferListing = json_object_get(data, "ployOfferListing");
	json_t *ployOffers = json_object_get(ployOfferListing, "ployOffers");

    tileCount = 0;
    size_t index;
    json_t *tile;
    max_scroll = 0;
    json_array_foreach(ployOffers, index, tile) {
        if (tileCount >= json_array_size(ployOffers)) break;

		json_t *coupon = json_object_get(tile, "coupon");
		json_t *targetPromotion = json_object_get(coupon, "targetPromotion");
        json_t *name = json_object_get(targetPromotion, "name");
        const char *text = json_string_value(name);

        if (text) {

            C2D_TextFontParse(&g_kuponText[tileCount], font[0], kupon_text_Buf, text);
            C2D_TextOptimize(&g_kuponText[tileCount]);

            buttonsy[5 + tileCount] = (Button){
                .x = 0,
                .y = 0,
                .width = 134,
                .height = 179,
                .imageNormal = couponbutton,
                .imagePressed = couponbutton_pressed,
                .isPressed = false,
                .scene = 14,
                .scene2 = 15,
                .scene3 = 15,
                .scene4 = 15,
				.scene5 = 15,
                .sizenmachen = 1.0f,
                .onClick = updateprom
            };

			if (text) {
				strncpy(tileNames[tileCount], text, sizeof(tileNames[tileCount]) - 1);
				tileNames[tileCount][sizeof(tileNames[tileCount]) - 1] = '\0';
			}

            log_to_file("Tile %zu: %s", index, tileNames[tileCount]);
            log_to_file("Button %zu", 5 + tileCount);
			max_scroll += buttonsy[5 + tileCount].width + 8;
            tileCount++;
        }
    }
	max_scroll -= 134;
    json_decref(response_root);

    przycskmachen = true;
    categoryfeeddone = true;
	loadingshit = false;
	json_done = true;
}
void getcard(const char* mejntoken, const char* refrenentokenenkurwen) {

    json_t *rootn = json_object();
    json_t *variablesn = json_object();
    
	json_object_set_new(rootn, "operationName", json_string("PaymentCards"));
    json_object_set_new(rootn, "query", json_string("query PaymentCards { paymentCards { paymentCards { id isDefault lastFourDigits } } }"));
    json_object_set_new(rootn, "variables", json_object());

    char *json_datan = json_dumps(rootn, JSON_COMPACT);
    json_decref(rootn);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Id: a531998ec966db0951239efb91519560346cfecac77459fe3b85c5b786fa41de");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Name: LoyaltyPoints");
    headers = curl_slist_append(headers, "Accept: application/json");

    char auth_headeren[1024];
    snprintf(auth_headeren, sizeof(auth_headeren), "Authorization: Bearer %s", mejntoken);
    headers = curl_slist_append(headers, auth_headeren);

    LightEvent response_event;
    LightEvent_Init(&response_event, RESET_ONESHOT);

    refresh_data("https://api.spapp.zabka.pl/", json_datan, headers);
	json_t *response_root = json_loads(global_response.data, 0, NULL);

    json_t *datas = json_object_get(response_root, "data");
    json_t *paymentCardsWrapper = json_object_get(datas, "paymentCards");
    json_t *paymentCards = json_object_get(paymentCardsWrapper, "paymentCards");

    if (json_is_array(paymentCards) && json_array_size(paymentCards) > 0) {
		json_t *fajlroot = json_load_file("/3ds/data.json", 0, NULL);
		json_object_set_new(fajlroot, "has_nano", json_string("yes"));
		json_dump_file(fajlroot, "/3ds/data.json", JSON_COMPACT);
		json_decref(fajlroot);
		usernan = "yes";
    } else {
		json_t *fajlroot = json_load_file("/3ds/data.json", 0, NULL);
		json_object_set_new(fajlroot, "has_nano", json_string("no"));
		json_dump_file(fajlroot, "/3ds/data.json", JSON_COMPACT);
		json_decref(fajlroot);
		usernan = "no";
    }
}
void getzappsy_startup(const char* mejntoken, const char* refrenentokenenkurwen) {
	consoleClear();
	const char* msg = "Pobieranie potrzebnych danych...";
	int x = (50 - strlen(msg)) / 2;
	int y = 30 / 2;
	printf("\x1b[%d;%dH%s", y, x, msg);  // ANSI escape to move cursor to (y, x)
    json_t *rootn = json_object();
    json_t *variablesn = json_object();
    
    json_object_set_new(rootn, "operationName", json_string("LoyaltyPoints"));
    json_object_set_new(rootn, "query", json_string("query LoyaltyPoints { loyaltyProgram { points pointsStatus pointsOperationsAvailable } }"));
    json_object_set_new(rootn, "variables", variablesn);

    char *json_datan = json_dumps(rootn, JSON_COMPACT);
    json_decref(rootn);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Id: a531998ec966db0951239efb91519560346cfecac77459fe3b85c5b786fa41de");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Name: LoyaltyPoints");
    headers = curl_slist_append(headers, "Accept: application/json");

    char auth_headeren[1024];
    snprintf(auth_headeren, sizeof(auth_headeren), "Authorization: Bearer %s", mejntoken);
    headers = curl_slist_append(headers, auth_headeren);

    LightEvent response_event;
    LightEvent_Init(&response_event, RESET_ONESHOT);

    refresh_data("https://api.spapp.zabka.pl/", json_datan, headers);
	json_t *response_root = json_loads(global_response.data, 0, NULL);
	if (response_root) {
		json_t *data = json_object_get(response_root, "data");
		if (data) {
			json_t *loyalty = json_object_get(data, "loyaltyProgram");
			if (loyalty) {
				json_t *punkty = json_object_get(loyalty, "points");
				if (punkty) {
					int amountzappsy = json_integer_value(punkty);
					sprintf(zappsystr, "%d", amountzappsy);
					log_to_file("Points: %d", amountzappsy);
				} else {
					log_to_file("ERROR: 'points' not found in the loyaltyProgram.");
				}
			} else {
				log_to_file("ERROR: 'loyaltyProgram' not found in the data.");
			}
		} else {
			log_to_file("ERROR: 'data' not found in the response.");
		}

		json_decref(response_root);
	} else {
		log_to_file("ERROR: Failed to parse JSON response.");
	}
}
void updatezappsy(const char* mejntoken, const char* refrenentokenenkurwen) {

    json_t *rootn = json_object();
    json_t *variablesn = json_object();
    
    json_object_set_new(rootn, "operationName", json_string("LoyaltyPoints"));
    json_object_set_new(rootn, "query", json_string("query LoyaltyPoints { loyaltyProgram { points pointsStatus pointsOperationsAvailable } }"));
    json_object_set_new(rootn, "variables", variablesn);

    char *json_datan = json_dumps(rootn, JSON_COMPACT);
    json_decref(rootn);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Id: a531998ec966db0951239efb91519560346cfecac77459fe3b85c5b786fa41de");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Name: LoyaltyPoints");
    headers = curl_slist_append(headers, "Accept: application/json");

    char auth_headeren[1024];
    snprintf(auth_headeren, sizeof(auth_headeren), "Authorization: Bearer %s", mejntoken);
    headers = curl_slist_append(headers, auth_headeren);

    LightEvent response_event;
    LightEvent_Init(&response_event, RESET_ONESHOT);

    char *local_response = NULL;
	size_t response_size = 0;

    log_to_file("Queuing request...");
    queue_request("https://api.spapp.zabka.pl/", json_datan, headers, (void **)&local_response, &response_size, &response_event, false);
}

void updateploy(const char* mejntoken, const char* refrenentokenenkurwen) {

    json_t *rootn = json_object();
    json_t *variablesn = json_object();
    
    json_object_set_new(rootn, "operationName", json_string("ZoneFeedByKey"));
    json_object_set_new(rootn, "query", json_string("query ZoneFeedByKey($zoneFeedKey: String!, $after: String) { zoneFeed(zoneFeedKey: $zoneFeedKey, after: $after) { sections { __typename ... on OfferTileSection { __typename ...OfferTileSectionParts } ... on PromotionBannerCarousel { __typename ...PromotionBannerCarouselParts } ... on ProductPromotionListingCarousel { __typename ...ProductPromotionListingCarouselParts } ... on PloyOfferListingCarousel { __typename ...PloyOfferListingCarouselParts } ... on DealCarousel { __typename ...DealCarouselParts } ... on OptionTileSection { __typename ...OptionTileSectionParts } ... on StoriesSection { __typename ...StoriesSectionParts } ... on DealListingCarousel { __typename ...DealListingCarouselParts } } pagination { cursor } } }  fragment OfferTileSectionParts on OfferTileSection { id title tiles { id backgroundImage { url } offerTreeLevel { __typename ... on OfferCategory { id name } ... on PloyOfferListing { id title } ... on ProductPromotionListing { id title } } } }  fragment SwipeCarouselPropertiesParts on SwipeCarouselProperties { swipeId: id swipeTitle: title }  fragment AutoplayCarouselPropertiesParts on AutoplayCarouselProperties { autoplayId: id autoplayTitle: title autoplayIntervalMillis }  fragment PriceParts on Price { amount currencyCode fractionDigits }  fragment ColorParts on ThemeColor { light dark }  fragment BadgeParts on PromotionBadge { key label textColor { __typename ...ColorParts } }  fragment AbsoluteDiscountValueParts on AbsoluteDiscountValue { absoluteValue: value { __typename ...PriceParts } absoluteDiscount: discount { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment RelativeDiscountValueParts on RelativeDiscountValue { percentage relativeValue: value { __typename ...PriceParts } omnibusLongDescription omnibusShortDescription roundOff }  fragment MultibuyDiscountValueParts on MultibuyDiscountValue { multibuyValue: value { __typename ...PriceParts } maxQuantity triggerQuantity multibuyDiscount: discount { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } } }  fragment DiscountParts on DiscountValue { __typename ... on AbsoluteDiscountValue { __typename ...AbsoluteDiscountValueParts } ... on RelativeDiscountValue { __typename ...RelativeDiscountValueParts } ... on MultibuyDiscountValue { __typename ...MultibuyDiscountValueParts } }  fragment HappyHourParts on HappyHour { alert validFrom validUntil }  fragment PromotionHighlightParts on PromotionHighlight { layout keyVisualImage { url } }  fragment TagParts on PromotionTag { backgroundColor { __typename ...ColorParts } key label longLabel textColor { __typename ...ColorParts } }  fragment ProductPromotionParts on ProductPromotion { id name image { url } detailsImage { url } contents detailsContents exclusivity isPricePerUnit hasLegalDetails hidePromotionAlerts legalShortDetails legalLongDetails validFrom validUntil promotionDurationDetails alternativeBasePrice { __typename ...PriceParts } basePrice { __typename ...PriceParts } badges { __typename ...BadgeParts } discount { __typename ...DiscountParts } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } tags { __typename ...TagParts } __typename }  fragment MediumPromotionBannerPropertiesParts on MediumPromotionBannerProperties { mediumTitle: title mediumSubtitle: subtitle image { url } }  fragment RelativeProductPromotionPriceLabelParts on RelativeProductPromotionPriceLabel { percentage roundOff textColor { __typename ...ColorParts } }  fragment AbsoluteProductPromotionPriceLabelParts on AbsoluteProductPromotionPriceLabel { price { __typename ...PriceParts } roundOff textColor { __typename ...ColorParts } }  fragment LargeProductPromotionListingBannerPropertiesParts on LargeProductPromotionListingBannerProperties { image { url } }  fragment DigitalProductPromotionParts on DigitalProductPromotion { id name code contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment PartnerProductPromotionParts on PartnerProductPromotion { id name contents detailsContents exclusivity images { url } happyHour { __typename ...HappyHourParts } highlight { __typename ...PromotionHighlightParts } badges { __typename ...BadgeParts } validFrom validUntil }  fragment InAppProductPromotionParts on InAppProductPromotion { id name contents detailsContents happyHour { __typename ...HappyHourParts } productType image { url } validFrom validUntil }  fragment CouponParts on Coupon { id keyVisualImage { url } lastingAt validFrom validUntil showFullDate targetPromotion { __typename ... on ProductPromotion { __typename ...ProductPromotionParts id } ... on DigitalProductPromotion { __typename ...DigitalProductPromotionParts } ... on PartnerProductPromotion { __typename ...PartnerProductPromotionParts } ... on InAppProductPromotion { __typename ...InAppProductPromotionParts } } currentRedeemedQuantity possibleRedeems redeemLimitPerClient state couponSource __typename }  fragment PloyOfferParts on PloyOffer { id coupon { __typename ...CouponParts id } price { base discounted } visualDiscount __typename }  fragment LargePloyOfferBannerPropertiesParts on LargePloyOfferBannerProperties { image { url } }  fragment MediumPloyOfferBannerPropertiesParts on MediumPloyOfferBannerProperties { image { url } }  fragment PloyOfferPriceLabelParts on PloyOfferPriceLabel { ployPrice roundOff textColor { __typename ...ColorParts } }  fragment CtaParts on Cta { title subtitle action { label link } id image { url } __typename }  fragment LargePromotionBannerPropertiesParts on LargePromotionBannerProperties { largeTitle: title largeSubtitle: subtitle image { url } }  fragment DealParts on Deal { id coupon { __typename ...CouponParts id } ployPrice __typename }  fragment QuestParts on Quest { id title subtitle bannerImage { url } detailsImage { url } lastingAt description questCompleted repeatable badges progressSummary { status totalSteps finishedSteps } stages { status subStages { status description totalSteps finishedSteps } completionDescription } }  fragment PromotionBannerCarouselParts on PromotionBannerCarousel { id title carouselProperties: properties { __typename ... on SwipeCarouselProperties { __typename ...SwipeCarouselPropertiesParts } ... on AutoplayCarouselProperties { __typename ...AutoplayCarouselPropertiesParts } } banners { __typename ... on ProductPromotionBanner { id productPromotion { __typename ...ProductPromotionParts id } properties { __typename ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } } ... on ProductPromotionListingBanner { id header title validFrom validUntil exclusivity priceLabel { __typename ... on RelativeProductPromotionPriceLabel { __typename ...RelativeProductPromotionPriceLabelParts } ... on AbsoluteProductPromotionPriceLabel { __typename ...AbsoluteProductPromotionPriceLabelParts } } listingRef: productPromotionListingRef { id } properties { __typename ... on LargeProductPromotionListingBannerProperties { __typename ...LargeProductPromotionListingBannerPropertiesParts } } } ... on PloyOfferBanner { id ployOffer { __typename ...PloyOfferParts id } properties { __typename ... on LargePloyOfferBannerProperties { __typename ...LargePloyOfferBannerPropertiesParts } ... on MediumPloyOfferBannerProperties { __typename ...MediumPloyOfferBannerPropertiesParts } } } ... on PloyOfferListingBanner { id header title validFrom validUntil exclusivity ployOfferListingRef { id } priceLabel { __typename ...PloyOfferPriceLabelParts } properties { __typename ... on LargePloyOfferBannerProperties { __typename ...LargePloyOfferBannerPropertiesParts } } } ... on CtaBanner { id cta { __typename ...CtaParts id } properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } header label } ... on Deal { __typename ...DealParts id } ... on LinkBanner { id header link properties { __typename ... on LargePromotionBannerProperties { __typename ...LargePromotionBannerPropertiesParts } ... on MediumPromotionBannerProperties { __typename ...MediumPromotionBannerPropertiesParts } } } ... on Quest { __typename ...QuestParts } } }  fragment ScrollCarouselPropertiesParts on ScrollCarouselProperties { id title showMore }  fragment ProductPromotionListingParts on ProductPromotionListing { id title productPromotions { __typename ...ProductPromotionParts id } pagination { cursor } }  fragment ProductPromotionListingCarouselParts on ProductPromotionListingCarousel { id title properties { __typename ...ScrollCarouselPropertiesParts } productPromotionListing { __typename ...ProductPromotionListingParts } }  fragment PloyOfferListingParts on PloyOfferListing { id title ployOffers { __typename ...PloyOfferParts id } pagination { cursor } }  fragment PloyOfferListingCarouselParts on PloyOfferListingCarousel { id properties { __typename ...ScrollCarouselPropertiesParts } ployOfferListing { __typename ...PloyOfferListingParts } }  fragment DealCarouselParts on DealCarousel { id deals { __typename ...DealParts id } properties { __typename ...ScrollCarouselPropertiesParts } }  fragment OptionTileParts on OptionTile { id label link }  fragment OptionTileSectionParts on OptionTileSection { id title subtitle optionTiles { __typename ...OptionTileParts } }  fragment StoriesSectionParts on StoriesSection { id title storiesSource labels }  fragment DealListingCarouselParts on DealListingCarousel { id dealListing { id title deals { __typename ...DealParts id } } properties { __typename ...ScrollCarouselPropertiesParts } }"));
    json_object_set_new(rootn, "variables", variablesn);
	json_object_set_new(variablesn, "zoneFeedKey", json_string("ploy-zabka"));

    char *json_datan = json_dumps(rootn, JSON_COMPACT);
    json_decref(rootn);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");

    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");

    char auth_headeren[1024];
    snprintf(auth_headeren, sizeof(auth_headeren), "Authorization: Bearer %s", mejntoken);
    headers = curl_slist_append(headers, auth_headeren);

    LightEvent response_event;
    LightEvent_Init(&response_event, RESET_ONESHOT);

    char *local_response = NULL;
	size_t response_size = 0;

    log_to_file("Queuing request...");
    queue_request("https://api.spapp.zabka.pl/", json_datan, headers, (void **)&local_response, &response_size, &response_event, false);
}

void sprawdzajtokenasa(const char* mejntoken, const char* refrenentokenenkurwen) {
	const char* msg = "Sprawdzanie poprawnosci tokena...";
	int x = (50 - strlen(msg)) / 2;
	int y = 30 / 2;
	printf("\x1b[%d;%dH%s", y, x, msg);  // ANSI escape to move cursor to (y, x)
    json_t *rootn = json_object();
    json_t *variablesn = json_object();

    json_object_set_new(rootn, "operationName", json_string("LoyaltyPoints"));
    json_object_set_new(rootn, "query", json_string("query LoyaltyPoints { loyaltyProgram { points pointsStatus pointsOperationsAvailable } }"));
    json_object_set_new(rootn, "variables", variablesn);

    char *json_datan = json_dumps(rootn, JSON_COMPACT);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Id: a531998ec966db0951239efb91519560346cfecac77459fe3b85c5b786fa41de");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Name: LoyaltyPoints");
    headers = curl_slist_append(headers, "Accept: application/json");
    char auth_headeren[1024];
    snprintf(auth_headeren, sizeof(auth_headeren), "Authorization: Bearer %s", mejntoken);
    headers = curl_slist_append(headers, auth_headeren);

    refresh_data("https://api.spapp.zabka.pl/", json_datan, headers);

    free(json_datan);
    curl_slist_free_all(headers);
    headers = NULL;
    json_object_clear(rootn);

	if (response_code != 0){
		if (strstr(global_response.data, "points") == NULL) {

			json_object_set_new(rootn, "grantType", json_string("refresh_token"));
			json_object_set_new(rootn, "refreshToken", json_string(refrenentokenenkurwen));
			json_datan = json_dumps(rootn, JSON_COMPACT);

			headers = curl_slist_append(headers, "Content-Type: application/json");

			refresh_data("https://securetoken.googleapis.com/v1/token?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", json_datan, headers);

			free(json_datan);

			json_object_clear(rootn);

			if (global_response.data) {
				json_t *response_root = json_loads(global_response.data, 0, NULL);
				if (response_root) {
					json_t *refresh_tokent = json_object_get(response_root, "refresh_token");
					json_t *access_tokent = json_object_get(response_root, "access_token");
				
					id_tokenk = strdup(json_string_value(access_tokent));
					refreshtoken = strdup(json_string_value(refresh_tokent));
					czas_wygasniecia = dawaj_expire_time(id_tokenk);
					
					json_decref(response_root);
					json_t *fajlroot = json_load_file("/3ds/data.json", 0, NULL);
					json_object_set_new(fajlroot, "token", json_string(id_tokenk));
					json_object_set_new(fajlroot, "refresh", json_string(refreshtoken));
					json_dump_file(fajlroot, "/3ds/data.json", JSON_COMPACT);
					json_decref(fajlroot);
				}
			}
			getcard(id_tokenk, refreshtoken);
			getzappsy_startup(id_tokenk, refreshtoken);
			curl_slist_free_all(headers);
			headers = NULL;
		} else {
			czas_wygasniecia = dawaj_expire_time(mejntoken);
			getzappsy_startup(id_tokenk, refreshtoken);
			getcard(id_tokenk, refreshtoken);
		}
	}
}

void login_flow(const char *phone_number, const char *verification_code) {

    json_t *root = json_object();
    json_t *variables = json_object();
    json_t *input = json_object();
    json_t *phone_number_obj = json_object();
	json_t *saveroot = json_object();

    json_object_set_new(phone_number_obj, "countryCode", json_string("48"));
    json_object_set_new(phone_number_obj, "nationalNumber", json_string(phone_number));
    json_object_set_new(input, "phoneNumber", phone_number_obj);
    json_object_set_new(input, "verificationCode", json_string(verification_code));
    json_object_set_new(variables, "input", input);
    json_object_set_new(root, "operationName", json_string("SignInWithPhone"));
    json_object_set_new(root, "query", json_string("mutation SignInWithPhone($input: SignInInput!) { signIn(input: $input) { customToken } }"));
    json_object_set_new(root, "variables", variables);

    char *json_data = json_dumps(root, JSON_COMPACT);


    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: okhttp/4.12.0");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Id: a531998ec966db0951239efb91519560346cfecac77459fe3b85c5b786fa41de");
    headers = curl_slist_append(headers, "X-Apollo-Operation-Name: SignInWithPhone");
    headers = curl_slist_append(headers, "Accept: application/json");
    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", id_tokenk);
    headers = curl_slist_append(headers, auth_header);


    refresh_data("https://super-account.spapp.zabka.pl/", json_data, headers);


    free(json_data);

	if (!youfuckedup) {
		json_t *response_root = json_loads(global_response.data, 0, NULL);
		if (response_root) {
			json_t *data = json_object_get(response_root, "data");
			json_t *sign_in = json_object_get(data, "signIn");
			json_t *custom_token_json = json_object_get(sign_in, "customToken");
			id_tokenk = strdup(json_string_value(custom_token_json));

			json_decref(response_root);
		}
		
		curl_slist_free_all(headers);
		headers = NULL; 
		json_object_clear(root);
		response_root = NULL;
		
		headers = curl_slist_append(headers, "Content-Type: application/json");

		json_object_set_new(root, "token", json_string(id_tokenk));
		json_object_set_new(root, "returnSecureToken", json_string("true"));
		json_data = json_dumps(root, JSON_COMPACT);


		refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/verifyCustomToken?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", json_data, headers);


		response_root = json_loads(global_response.data, 0, NULL);
		if (response_root) {
			json_t *custom_token_json = json_object_get(response_root, "idToken");
			json_t *refresh_token_json = json_object_get(response_root, "refreshToken");
			if (custom_token_json && json_is_string(custom_token_json)) {
				id_tokenk = strdup(json_string_value(custom_token_json));
				czas_wygasniecia = dawaj_expire_time(id_tokenk);
				refreshtoken = strdup(json_string_value(refresh_token_json));
				printf("Extracted idToken: %s\n", id_tokenk);
			} else {
				fprintf(stderr, "Error: idToken not found.\n");
				id_tokenk = NULL;
			}
			json_decref(response_root);
		} 

		if (id_tokenk) {
			root = json_object();
			json_object_set_new(root, "idToken", json_string(id_tokenk));
			json_object_set_new(saveroot, "token", json_string(id_tokenk));
			json_object_set_new(saveroot, "refresh", json_string(refreshtoken));
			json_data = json_dumps(root, JSON_COMPACT);

			refresh_data("https://www.googleapis.com/identitytoolkit/v3/relyingparty/getAccountInfo?key=AIzaSyDe2Fgxn_8HJ6NrtJtp69YqXwocutAoa9Q", json_data, headers);
		}

		free(json_data);
		curl_slist_free_all(headers);
		headers = NULL; 
		json_object_clear(root);
		
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "accept: application/json");
		headers = curl_slist_append(headers, "user-agent: Zappka/40038 (Horizon; nintendo/ctr; 56c41945-ba88-4543-a525-4e8f7d4a5812) REL/28");
		snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", id_tokenk);
		headers = curl_slist_append(headers, auth_header);

		variables = json_object();
		json_t *signInInput = json_object();

		json_object_set_new(signInInput, "sessionId", json_string("65da013a-0d7d-3ad4-82bd-2bc15077d7f5"));
		json_object_set_new(variables, "signInInput", signInInput);

		json_object_set_new(root, "operationName", json_string("SignIn"));
		json_object_set_new(root, "query", json_string("mutation SignIn($signInInput: SignInInput!) { signIn(signInInput: $signInInput) { profile { __typename ...UserProfileParts } } }  fragment UserProfileParts on UserProfile { email gender }"));
		json_object_set_new(root, "variables", variables);
		json_data = json_dumps(root, JSON_COMPACT);

		
		refresh_data("https://api.spapp.zabka.pl/", json_data, headers);

		json_t *verify_response = json_loads(global_response.data, 0, NULL);
		json_t *verify_data = json_object_get(verify_response, "data");
		json_t *verify_token = json_object_get(verify_data, "verifyCustomToken");
		json_t *id_token_json_resp = json_object_get(verify_token, "idToken");
		id_tokenk = json_string_value(id_token_json_resp);

		free(json_data);
		json_object_clear(root);


		json_object_set_new(root, "operationName", json_string("QrCode"));
		json_object_set_new(root, "query", json_string("query QrCode { qrCode { loyalSecret paySecret ployId } }"));
		json_object_set_new(root, "variables", json_object());
		
		json_data = json_dumps(root, JSON_COMPACT);
		refresh_data("https://api.spapp.zabka.pl/", json_data, headers);
		
		response_root = json_loads(global_response.data, 0, NULL);
		json_t *data = json_object_get(response_root, "data");
		json_t *qrCode = json_object_get(data, "qrCode");
		json_t *ployId = json_object_get(qrCode, "ployId");
		json_t *loyalSecret = json_object_get(qrCode, "loyalSecret");
		json_t *paymentSecret = json_object_get(qrCode, "paySecret");
		char *user_id = strdup(json_string_value(ployId));
		char *hex_secret = strdup(json_string_value(loyalSecret));
		char *pay_secret = strdup(json_string_value(paymentSecret));
		
		
		userajd = user_id;
		json_object_set_new(saveroot, "user_id", json_string(user_id));
		json_object_set_new(saveroot, "hex_secret", json_string(hex_secret));
		json_object_set_new(saveroot, "pay_secret", json_string(pay_secret));
		
		
		
		free(json_data);
		json_object_clear(root);

		json_object_set_new(root, "operationName", json_string("PaymentCards"));
		json_object_set_new(root, "query", json_string("query PaymentCards { paymentCards { paymentCards { id isDefault lastFourDigits } } }"));
		json_object_set_new(root, "variables", json_object());
		
		json_data = json_dumps(root, JSON_COMPACT);
		refresh_data("https://api.spapp.zabka.pl/", json_data, headers);
		response_root = json_loads(global_response.data, 0, NULL);

		json_t *datas = json_object_get(response_root, "data");
		json_t *paymentCardsWrapper = json_object_get(datas, "paymentCards");
		json_t *paymentCards = json_object_get(paymentCardsWrapper, "paymentCards");

		if (json_is_array(paymentCards) && json_array_size(paymentCards) > 0) {
			secrettotpglobal = pay_secret;
			json_object_set_new(saveroot, "has_nano", json_string("yes"));
			usernan = "yes";
		} else {
			secrettotpglobal = hex_secret;
			json_object_set_new(saveroot, "has_nano", json_string("no"));
			usernan = "no";
		}

		json_object_clear(root);
		
		
		json_object_set_new(root, "operationName", json_string("GetProfile"));
		json_object_set_new(root, "query", json_string("query GetProfile { profile { id firstName birthDate phoneNumber { countryCode nationalNumber } email } }"));
		json_object_set_new(root, "variables", json_object());
		
		json_data = json_dumps(root, JSON_COMPACT);
		refresh_data("https://super-account.spapp.zabka.pl/", json_data, headers);
		response_root = json_loads(global_response.data, 0, NULL);
		json_t *nejmdata = json_object_get(response_root, "data");
		json_t *profajl = json_object_get(nejmdata, "profile");
		json_t *nejm = json_object_get(profajl, "firstName");
		char *nejmjson = strdup(json_string_value(nejm));
		json_object_set_new(saveroot, "name", json_string(nejmjson));
		char *save_data = json_dumps(saveroot, JSON_COMPACT);

		
	  
		free(json_data);
		json_decref(root);
		json_decref(response_root);
		fclose(fptr);
		FILE *log_file = fopen("/3ds/data.json", "w");
		fprintf(log_file, save_data);
		fclose(log_file);
		json_decref(saveroot);
		json_t *jsonfl;
		jsonfl = json_load_file("/3ds/data.json", 0, NULL);
		json_t *ajdentokenen = json_object_get(jsonfl, "token");
		id_tokenk = json_string_value(ajdentokenen);
		json_t *nejmen = json_object_get(jsonfl, "name");
		nejmenmachen = json_string_value(nejmen);
		snprintf(combinedText, sizeof(combinedText), "Witaj %s!", nejmenmachen);
		C2D_TextFontParse(&g_staticText[7], font[0], g_staticBuf, combinedText);
		getzappsy_startup(id_tokenk, refreshtoken);
	}
}