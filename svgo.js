module.exports = {
  multipass: true,
  plugins: [
    "cleanupIDs",
    // "mergeStyles",
    {
      name: "mergePaths",
      params: {
        force: true,
      },
    },
    // "minifyStyles",
    {
      name: "convertPathData",
      params: {
        transformPrecision: 0,
        floatPrecision: 0,
        removeUseless: true,
        utilizeAbsolute: false,
        forceAbsolutePath: false,
        curveSmoothShorthands: false,
        negativeExtraSpace: true,
      },
    },
  ],
};
