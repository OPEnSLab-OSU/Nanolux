import { h } from 'preact';

// Note: `user` comes from the URL, courtesy of our router
const Profile = ({ user }) => {
	return (
		<div>
			<h1>{user}</h1>
			<p>This will contain controls to manage user authentication.</p>
		</div>
	);
};

export default Profile;
