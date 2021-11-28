#pragma once

#include "common.hpp"

namespace Gfx {

struct Material {
    Real reflectance{}; //1 means perfect mirror
    Real ior{}; //index of refraction

    Point albedo{};
    Point emittance{};
};

enum class MaterialPreset {
    Acetone,
    Actinolite,
    Agalmatoite,
    Agate,
    AgateMoss,
    Air,
    Alcohol,
    Alexandrite,
    Aluminum,
    Amber,
    Amblygonite,
    Amethyst,
    Anatase,
    Andalusite,
    Anhydrite,
    Apatite,
    Apophyllite,
    Aquamarine,
    Aragonite,
    Argon,
    Asphalt,
    Augelite,
    Axinite,
    Azurite,
    Barite,
    Barytocalcite,
    Benitoite,
    Benzene,
    Beryl,
    Beryllonite,
    Brazilianite,
    BromineLiquid,
    Bronze,
    Brownite,
    Calcite,
    Calspar,
    Cancrinite,
    CarbonDioxideGas,
    CarbonDisulfide,
    CarbonTetrachloride,
    Cassiterite,
    Celestite,
    Cerussite,
    Ceylanite,
    Chalcedony,
    Chalk,
    Chalybite,
    ChlorineGas,
    ChlorineLiquid,
    ChromeGreen,
    ChromeRed,
    ChromeYellow,
    Chromium,
    Chrysoberyl,
    Chrysocolla,
    Chrysoprase,
    Citrine,
    Clinozoisite,
    CobaltBlue,
    CobaltGreen,
    CobaltViolet,
    Colemanite,
    Copper,
    CopperOxide,
    Coral,
    Cordierite,
    Corundum,
    Crocoite,
    Crystal,
    Cuprite,
    Danburite,
    Diamond,
    Diopside,
    Dolomite,
    Dumortierite,
    Ebonite,
    Ekanite,
    Elaeolite,
    Emerald,
    EmeraldSynthFlux,
    EmeraldSynthHydro,
    Enstatite,
    Epidote,
    Ethanol,
    EthylAlcohol,
    Euclase,
    Fabulite,
    FeldsparAdventurine,
    FeldsparAlbite,
    FeldsparAmazonite,
    FeldsparLabradorite,
    FeldsparMicrocline,
    FeldsparOligoclase,
    FeldsparOrthoclase,
    Fluoride,
    Fluorite,
    Formica,
    GarnetAlmandine,
    GarnetAlmandite,
    GarnetAndradite,
    GarnetDemantoid,
    GarnetGrossular,
    GarnetHessonite,
    GarnetRhodolite,
    GarnetSpessartite,
    Gaylussite,
    Glass,
    GlassAlbite,
    GlassCrown,
    GlassCrownZinc,
    GlassFlintDense,
    GlassFlintHeaviest,
    GlassFlintHeavy,
    GlassFlintLanthanum,
    GlassFlintLight,
    GlassFlintMedium,
    Glycerine,
    Gold,
    Hambergite,
    Hauynite,
    Helium,
    Hematite,
    Hemimorphite,
    Hiddenite,
    Howlite,
    HydrogenGas,
    HydrogenLiquid,
    Hypersthene,
    Ice,
    Idocrase,
    IodineCrystal,
    Iolite,
    Iron,
    Ivory,
    JadeNephrite,
    Jadeite,
    Jasper,
    Jet,
    Kornerupine,
    Kunzite,
    Kyanite,
    LapisGem,
    LapisLazuli,
    Lazulite,
    Lead,
    Leucite,
    Magnesite,
    Malachite,
    Meerschaum,
    MercuryLiquid,
    Methanol,
    Moldavite,
    MoonstoneAdularia,
    MoonstoneAlbite,
    Natrolite,
    Nephrite,
    NitrogenGas,
    NitrogenLiquid,
    Nylon,
    Obsidian,
    Olivine,
    Onyx,
    Opal,
    OxygenGas,
    OxygenLiq,
    Painite,
    Pearl,
    Periclase,
    Peridot,
    Peristerite,
    Petalite,
    Phenakite,
    Phosgenite,
    Plastic,
    Plexiglas,
    Polystyrene,
    Prase,
    Prasiolite,
    Prehnite,
    Proustite,
    Purpurite,
    Pyrite,
    Pyrope,
    Quartz,
    QuartzFused,
    Rhodizite,
    Rhodochrisite,
    Rhodonite,
    RockSalt,
    RubberNatural,
    Ruby,
    Rutile,
    Sanidine,
    Sapphire,
    Scapolite,
    ScapoliteYellow,
    Scheelite,
    SeleniumAmorphous,
    Serpentine,
    Shell,
    Silicon,
    Sillimanite,
    Silver,
    Sinhalite,
    Smaragdite,
    Smithsonite,
    Sodalite,
    SodiumChloride,
    Sphalerite,
    Sphene,
    Spinel,
    Spodumene,
    Staurolite,
    Steatite,
    Steel,
    Stichtite,
    StrontiumTitanate,
    Styrofoam,
    Sulphur,
    SyntheticSpinel,
    Taaffeite,
    Tantalite,
    Tanzanite,
    Teflon,
    Thomsonite,
    TigerEye,
    Topaz,
    TopazBlue,
    TopazPink,
    TopazWhite,
    TopazYellow,
    Tourmaline,
    Tremolite,
    Tugtupite,
    Turpentine,
    Turquoise,
    Ulexite,
    Uvarovite,
    Variscite,
    Vivianite,
    Wardite,
    WaterGas,
    Water100DegCelcius,
    Water20DegCelcius,
    Water35DegCelcius,
    Willemite,
    Witherite,
    Wulfenite,
    Zincite,
    ZirconHigh,
    ZirconLow,
    ZirconiaCubic,
};

static inline const std::unordered_map<MaterialPreset, Real> PresetIORs{
  {MaterialPreset::Acetone,             1.36},
  {MaterialPreset::Actinolite,          1.618},
  {MaterialPreset::Agalmatoite,         1.550},
  {MaterialPreset::Agate,               1.544},
  {MaterialPreset::AgateMoss,           1.540},
  {MaterialPreset::Air,                 1.0002926},
  {MaterialPreset::Alcohol,             1.329},
  {MaterialPreset::Alexandrite,         1.745},
  {MaterialPreset::Aluminum,            1.44},
  {MaterialPreset::Amber,               1.546},
  {MaterialPreset::Amblygonite,         1.611},
  {MaterialPreset::Amethyst,            1.544},
  {MaterialPreset::Anatase,             2.490},
  {MaterialPreset::Andalusite,          1.641},
  {MaterialPreset::Anhydrite,           1.571},
  {MaterialPreset::Apatite,             1.632},
  {MaterialPreset::Apophyllite,         1.536},
  {MaterialPreset::Aquamarine,          1.577},
  {MaterialPreset::Aragonite,           1.530},
  {MaterialPreset::Argon,               1.000281},
  {MaterialPreset::Asphalt,             1.635},
  {MaterialPreset::Augelite,            1.574},
  {MaterialPreset::Axinite,             1.675},
  {MaterialPreset::Azurite,             1.730},
  {MaterialPreset::Barite,              1.636},
  {MaterialPreset::Barytocalcite,       1.684},
  {MaterialPreset::Benitoite,           1.757},
  {MaterialPreset::Benzene,             1.501},
  {MaterialPreset::Beryl,               1.577},
  {MaterialPreset::Beryllonite,         1.553},
  {MaterialPreset::Brazilianite,        1.603},
  {MaterialPreset::BromineLiquid,       1.661},
  {MaterialPreset::Bronze,              1.18},
  {MaterialPreset::Brownite,            1.567},
  {MaterialPreset::Calcite,             1.486},
  {MaterialPreset::Calspar,             1.486},
  {MaterialPreset::Cancrinite,          1.491},
  {MaterialPreset::CarbonDioxideGas,    1.000449},
  {MaterialPreset::CarbonDisulfide,     1.628},
  {MaterialPreset::CarbonTetrachloride, 1.460},
  {MaterialPreset::Cassiterite,         1.997},
  {MaterialPreset::Celestite,           1.622},
  {MaterialPreset::Cerussite,           1.804},
  {MaterialPreset::Ceylanite,           1.770},
  {MaterialPreset::Chalcedony,          1.530},
  {MaterialPreset::Chalk,               1.510},
  {MaterialPreset::Chalybite,           1.630},
  {MaterialPreset::ChlorineGas,         1.000768},
  {MaterialPreset::ChlorineLiquid,      1.385},
  {MaterialPreset::ChromeGreen,         2.4},
  {MaterialPreset::ChromeRed,           2.42},
  {MaterialPreset::ChromeYellow,        2.31},
  {MaterialPreset::Chromium,            2.97},
  {MaterialPreset::Chrysoberyl,         1.745},
  {MaterialPreset::Chrysocolla,         1.500},
  {MaterialPreset::Chrysoprase,         1.534},
  {MaterialPreset::Citrine,             1.550},
  {MaterialPreset::Clinozoisite,        1.724},
  {MaterialPreset::CobaltBlue,          1.74},
  {MaterialPreset::CobaltGreen,         1.97},
  {MaterialPreset::CobaltViolet,        1.71},
  {MaterialPreset::Colemanite,          1.586},
  {MaterialPreset::Copper,              1.10},
  {MaterialPreset::CopperOxide,         2.705},
  {MaterialPreset::Coral,               1.486},
  {MaterialPreset::Cordierite,          1.540},
  {MaterialPreset::Corundum,            1.766},
  {MaterialPreset::Crocoite,            2.310},
  {MaterialPreset::Crystal,             2.00},
  {MaterialPreset::Cuprite,             2.850},
  {MaterialPreset::Danburite,           1.633},
  {MaterialPreset::Diamond,             2.417},
  {MaterialPreset::Diopside,            1.680},
  {MaterialPreset::Dolomite,            1.503},
  {MaterialPreset::Dumortierite,        1.686},
  {MaterialPreset::Ebonite,             1.66},
  {MaterialPreset::Ekanite,             1.600},
  {MaterialPreset::Elaeolite,           1.532},
  {MaterialPreset::Emerald,             1.576},
  {MaterialPreset::EmeraldSynthFlux,    1.561},
  {MaterialPreset::EmeraldSynthHydro,   1.568},
  {MaterialPreset::Enstatite,           1.663},
  {MaterialPreset::Epidote,             1.733},
  {MaterialPreset::Ethanol,             1.36},
  {MaterialPreset::EthylAlcohol,        1.36},
  {MaterialPreset::Euclase,             1.652},
  {MaterialPreset::Fabulite,            2.409},
  {MaterialPreset::FeldsparAdventurine, 1.532},
  {MaterialPreset::FeldsparAlbite,      1.525},
  {MaterialPreset::FeldsparAmazonite,   1.525},
  {MaterialPreset::FeldsparLabradorite, 1.565},
  {MaterialPreset::FeldsparMicrocline,  1.525},
  {MaterialPreset::FeldsparOligoclase,  1.539},
  {MaterialPreset::FeldsparOrthoclase,  1.525},
  {MaterialPreset::Fluoride,            1.56},
  {MaterialPreset::Fluorite,            1.434},
  {MaterialPreset::Formica,             1.47},
  {MaterialPreset::GarnetAlmandine,     1.760},
  {MaterialPreset::GarnetAlmandite,     1.790},
  {MaterialPreset::GarnetAndradite,     1.820},
  {MaterialPreset::GarnetDemantoid,     1.880},
  {MaterialPreset::GarnetGrossular,     1.738},
  {MaterialPreset::GarnetHessonite,     1.745},
  {MaterialPreset::GarnetRhodolite,     1.760},
  {MaterialPreset::GarnetSpessartite,   1.810},
  {MaterialPreset::Gaylussite,          1.517},
  {MaterialPreset::Glass,               1.51714},
  {MaterialPreset::GlassAlbite,         1.4890},
  {MaterialPreset::GlassCrown,          1.520},
  {MaterialPreset::GlassCrownZinc,      1.517},
  {MaterialPreset::GlassFlintDense,     1.66},
  {MaterialPreset::GlassFlintHeaviest,  1.89},
  {MaterialPreset::GlassFlintHeavy,     1.65548},
  {MaterialPreset::GlassFlintLanthanum, 1.80},
  {MaterialPreset::GlassFlintLight,     1.58038},
  {MaterialPreset::GlassFlintMedium,    1.62725},
  {MaterialPreset::Glycerine,           1.473},
  {MaterialPreset::Gold,                0.47},
  {MaterialPreset::Hambergite,          1.559},
  {MaterialPreset::Hauynite,            1.502},
  {MaterialPreset::Helium,              1.000036},
  {MaterialPreset::Hematite,            2.940},
  {MaterialPreset::Hemimorphite,        1.614},
  {MaterialPreset::Hiddenite,           1.655},
  {MaterialPreset::Howlite,             1.586},
  {MaterialPreset::HydrogenGas,         1.000140},
  {MaterialPreset::HydrogenLiquid,      1.0974},
  {MaterialPreset::Hypersthene,         1.670},
  {MaterialPreset::Ice,                 1.309},
  {MaterialPreset::Idocrase,            1.713},
  {MaterialPreset::IodineCrystal,       3.34},
  {MaterialPreset::Iolite,              1.548},
  {MaterialPreset::Iron,                1.51},
  {MaterialPreset::Ivory,               1.540},
  {MaterialPreset::JadeNephrite,        1.610},
  {MaterialPreset::Jadeite,             1.665},
  {MaterialPreset::Jasper,              1.540},
  {MaterialPreset::Jet,                 1.660},
  {MaterialPreset::Kornerupine,         1.665},
  {MaterialPreset::Kunzite,             1.655},
  {MaterialPreset::Kyanite,             1.715},
  {MaterialPreset::LapisGem,            1.500},
  {MaterialPreset::LapisLazuli,         1.61},
  {MaterialPreset::Lazulite,            1.615},
  {MaterialPreset::Lead,                2.01},
  {MaterialPreset::Leucite,             1.509},
  {MaterialPreset::Magnesite,           1.515},
  {MaterialPreset::Malachite,           1.655},
  {MaterialPreset::Meerschaum,          1.530},
  {MaterialPreset::MercuryLiquid,       1.62},
  {MaterialPreset::Methanol,            1.329},
  {MaterialPreset::Moldavite,           1.500},
  {MaterialPreset::MoonstoneAdularia,   1.525},
  {MaterialPreset::MoonstoneAlbite,     1.535},
  {MaterialPreset::Natrolite,           1.480},
  {MaterialPreset::Nephrite,            1.600},
  {MaterialPreset::NitrogenGas,         1.000297},
  {MaterialPreset::NitrogenLiquid,      1.2053},
  {MaterialPreset::Nylon,               1.53},
  {MaterialPreset::Obsidian,            1.489},
  {MaterialPreset::Olivine,             1.670},
  {MaterialPreset::Onyx,                1.486},
  {MaterialPreset::Opal,                1.450},
  {MaterialPreset::OxygenGas,           1.000276},
  {MaterialPreset::OxygenLiq,           1.221},
  {MaterialPreset::Painite,             1.787},
  {MaterialPreset::Pearl,               1.530},
  {MaterialPreset::Periclase,           1.740},
  {MaterialPreset::Peridot,             1.654},
  {MaterialPreset::Peristerite,         1.525},
  {MaterialPreset::Petalite,            1.502},
  {MaterialPreset::Phenakite,           1.650},
  {MaterialPreset::Phosgenite,          2.117},
  {MaterialPreset::Plastic,             1.460},
  {MaterialPreset::Plexiglas,           1.50},
  {MaterialPreset::Polystyrene,         1.55},
  {MaterialPreset::Prase,               1.540},
  {MaterialPreset::Prasiolite,          1.540},
  {MaterialPreset::Prehnite,            1.610},
  {MaterialPreset::Proustite,           2.790},
  {MaterialPreset::Purpurite,           1.840},
  {MaterialPreset::Pyrite,              1.810},
  {MaterialPreset::Pyrope,              1.740},
  {MaterialPreset::Quartz,              1.544},
  {MaterialPreset::QuartzFused,         1.45843},
  {MaterialPreset::Rhodizite,           1.690},
  {MaterialPreset::Rhodochrisite,       1.600},
  {MaterialPreset::Rhodonite,           1.735},
  {MaterialPreset::RockSalt,            1.544},
  {MaterialPreset::RubberNatural,       1.5191},
  {MaterialPreset::Ruby,                1.760},
  {MaterialPreset::Rutile,              2.62},
  {MaterialPreset::Sanidine,            1.522},
  {MaterialPreset::Sapphire,            1.760},
  {MaterialPreset::Scapolite,           1.540},
  {MaterialPreset::ScapoliteYellow,     1.555},
  {MaterialPreset::Scheelite,           1.920},
  {MaterialPreset::SeleniumAmorphous,   2.92},
  {MaterialPreset::Serpentine,          1.560},
  {MaterialPreset::Shell,               1.530},
  {MaterialPreset::Silicon,             4.24},
  {MaterialPreset::Sillimanite,         1.658},
  {MaterialPreset::Silver,              0.18},
  {MaterialPreset::Sinhalite,           1.699},
  {MaterialPreset::Smaragdite,          1.608},
  {MaterialPreset::Smithsonite,         1.621},
  {MaterialPreset::Sodalite,            1.483},
  {MaterialPreset::SodiumChloride,      1.544},
  {MaterialPreset::Sphalerite,          2.368},
  {MaterialPreset::Sphene,              1.885},
  {MaterialPreset::Spinel,              1.712},
  {MaterialPreset::Spodumene,           1.650},
  {MaterialPreset::Staurolite,          1.739},
  {MaterialPreset::Steatite,            1.539},
  {MaterialPreset::Steel,               2.50},
  {MaterialPreset::Stichtite,           1.520},
  {MaterialPreset::StrontiumTitanate,   2.410},
  {MaterialPreset::Styrofoam,           1.595},
  {MaterialPreset::Sulphur,             1.960},
  {MaterialPreset::SyntheticSpinel,     1.730},
  {MaterialPreset::Taaffeite,           1.720},
  {MaterialPreset::Tantalite,           2.240},
  {MaterialPreset::Tanzanite,           1.691},
  {MaterialPreset::Teflon,              1.35},
  {MaterialPreset::Thomsonite,          1.530},
  {MaterialPreset::TigerEye,            1.544},
  {MaterialPreset::Topaz,               1.620},
  {MaterialPreset::TopazBlue,           1.610},
  {MaterialPreset::TopazPink,           1.620},
  {MaterialPreset::TopazWhite,          1.630},
  {MaterialPreset::TopazYellow,         1.620},
  {MaterialPreset::Tourmaline,          1.624},
  {MaterialPreset::Tremolite,           1.600},
  {MaterialPreset::Tugtupite,           1.496},
  {MaterialPreset::Turpentine,          1.472},
  {MaterialPreset::Turquoise,           1.610},
  {MaterialPreset::Ulexite,             1.490},
  {MaterialPreset::Uvarovite,           1.870},
  {MaterialPreset::Variscite,           1.550},
  {MaterialPreset::Vivianite,           1.580},
  {MaterialPreset::Wardite,             1.590},
  {MaterialPreset::WaterGas,            1.000261},
  {MaterialPreset::Water100DegCelcius,  1.31819},
  {MaterialPreset::Water20DegCelcius,   1.33335},
  {MaterialPreset::Water35DegCelcius,   1.33157},
  {MaterialPreset::Willemite,           1.690},
  {MaterialPreset::Witherite,           1.532},
  {MaterialPreset::Wulfenite,           2.300},
  {MaterialPreset::Zincite,             2.010},
  {MaterialPreset::ZirconHigh,          1.960},
  {MaterialPreset::ZirconLow,           1.800},
  {MaterialPreset::ZirconiaCubic,       2.170},
};

}