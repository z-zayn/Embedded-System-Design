import js from "@eslint/js";
import vue from "eslint-plugin-vue";
import tseslint from "typescript-eslint";
import vueParser from "vue-eslint-parser";
import autoImportGlobals from "./.eslintrc-auto-import.json" assert { type: "json" };

export default [
  {
    ignores: ["dist/**", "build/**", "coverage/**", "node_modules/**"],
  },

  js.configs.recommended,
  ...tseslint.configs.recommended,

  // Vue（SFC）规则
  ...vue.configs["flat/recommended"],

  // 针对 .vue 做 parser 串联：vue-eslint-parser + typescript-eslint
  {
    files: ["**/*.vue"],
    languageOptions: {
      parser: vueParser,
      parserOptions: {
        parser: tseslint.parser,
        ecmaVersion: "latest",
        sourceType: "module",
        extraFileExtensions: [".vue"],
      },
      globals: {
        ...autoImportGlobals.globals,
      },
    },
    rules: {
      "vue/multi-word-component-names": "off",
    },
  },

  // 针对 .ts/.tsx
  {
    files: ["**/*.ts", "**/*.tsx"],
    languageOptions: {
      parser: tseslint.parser,
    },
    rules: {
      "@typescript-eslint/no-unused-vars": ["warn", { argsIgnorePattern: "^_" }],
      "vue/max-attributes-per-line": "off",
      "vue/first-attribute-linebreak": "off",
    },
  },
];
