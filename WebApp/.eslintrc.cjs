module.exports = {
  parser: '@babel/eslint-parser',
  parserOptions: {
    ecmaVersion: 2021,
    sourceType: 'module',
    ecmaFeatures: { jsx: true },
  },
  env: {
    browser: true,
    es2021: true,
    node: true,
  },
  extends: [
    'preact',
    'plugin:jsx-a11y/recommended'
  ],
  plugins: ['jsx-a11y'],
  rules: {
    'jsx-a11y/no-onchange': 'off'
  },
};
