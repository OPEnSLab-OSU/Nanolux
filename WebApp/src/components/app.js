import { Router } from 'preact-router';

import Header from './header';

// Code-splitting is automated for `routes` directory
import Settings from '../routes/sets';
import Profile from '../routes/user';
import Wifi from "../routes/wifi";


const App = () => (
	<div id="app">
		<Header />
		<main>
			<Router>
				<Settings path="/" />
				<Profile path="/user/" user="me" />
				<Wifi path="/wifi" />
			</Router>
		</main>
	</div>
);

export default App;
