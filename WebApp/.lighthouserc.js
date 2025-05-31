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
        'categories:accessibility': ['error', { minScore: 0.9 }]
      }
    }
  }
};
