module.exports = {
  ci: {
    collect: {
      startServerCommand: 'npm run build && npm run serve',
      readyPattern: 'Your application is ready',
      url: [
        'http://localhost:8080/',
        'http://localhost:8080/wifi'
      ],
      numberOfRuns: 1,
    },
    assert: {
      assertions: {
        // To adjust how strict the Lighthouse check is, uncomment or comment out any of the category lines below:
        // 'categories:performance':    ['error', { minScore: 0.9 }]
        'categories:accessibility':  ['error', { minScore: 0.9 }]
        // 'categories:best-practices': ['error', { minScore: 0.9 }]
        // 'categories:seo':            ['error', { minScore: 0.9 }]
      }
    }
  }
};
