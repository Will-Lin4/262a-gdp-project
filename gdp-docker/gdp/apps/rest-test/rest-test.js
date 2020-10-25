require.config({
    baseUrl: "./",
    paths: {
        jquery: "jquery",
        gdp: "gdp"
    }
});

require(["gdp"], function(gdp) {
    gdp.listGcl();
});
