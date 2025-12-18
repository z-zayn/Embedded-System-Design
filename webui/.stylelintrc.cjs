module.exports = {
  extends: [
    "stylelint-config-standard",
    "stylelint-config-standard-scss",
    "stylelint-config-recommended-vue",
    "stylelint-config-prettier"
  ],
  overrides: [
    {
      files: ["**/*.{vue,html}"],
      customSyntax: "postcss-html",
    },
  ],
  rules: {
    "selector-class-pattern": null,
  },
  ignoreFiles: ["dist/**", "build/**", "coverage/**", "node_modules/**"],
};
