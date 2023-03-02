import { Router } from 'preact-router';

import Header from './header';

// Code-splitting is automated for `rtes` directory
import Settings from '../rtes/setngs';
import Profile from '../rtes/profile';

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
