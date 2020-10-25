define(["jquery"], function($) {
    var restProtocol = "http";
    var host = "127.0.0.1";
    var gdpBaseUrl = restProtocol + "://" + host + "/gdp/v1";
    var gdpListGclUrl = gdpBaseUrl + "/gcl";

    function listGcl() {
        $.ajax({
            async: false,
            url: gdpListGclUrl,
            type: "GET",
            dataType: "json",
            error: function(jqXHR, textStatus, errorThrown) {
                console.log("listUsc $.ajax error: textStatus = " + textStatus +
                    ", errorThrown = " + errorThrown);
            },
            success: function(data, textStatus, jqXHR) {
                console.log("listUsc $.ajax success: textStatus = " + textStatus +
                    ", data = " + JSON.stringify(data));
            }
        });
    }

    return {
        listGcl: listGcl
    };
});
