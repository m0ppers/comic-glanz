module.exports = {
  multipass: true,
  plugins: [
    "cleanupIDs",
    // "mergeStyles",
    "mergePaths",
    // "minifyStyles",
    {
      name: "convertPathData",
      params: {
        transformPrecision: 0,
        floatPrecision: 0,
        removeUseless: true,
        forceAbsolutePath: true,
        curveSmoothShorthands: false,
      },
    },
  ],
};
