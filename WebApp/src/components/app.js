import { Router } from 'preact-router';

import Header from './header';

// Code-splitting is automated for `routes` directory
import Settings from '../routes/settings';
import Profile from '../routes/profile';


const App = () => (
	<div id="app">
		<Header />
		<main>
			<Router>
				<Settings path="/" />
				<Profile path="/profile/" user="me" />
				<Profile path="/profile/:user" />
			</Router>
		</main>
	</div>
);

export default App;
